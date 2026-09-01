// Christopher Naglik All Rights Reserved

#include "Framework/Flag.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GenericTeamAgentInterface.h"
#include "GAS/CHeroAttributeSet.h"
#include "GAS/ChrisAttributeSet.h"
#include "Framework/ChrisGameState.h"
#include "Net/UnrealNetwork.h"
#include "Player/ChrisPlayerCharacter.h"
#include "Audio/ChrisAudioSubsystem.h"
#include "Audio/ChrisGameplayTags.h"
#include "Character/ChrisCharacter.h"


AFlag::AFlag()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    bAlwaysRelevant = true;

    InfluenceRange = CreateDefaultSubobject<USphereComponent>("InfluenceRange");
    SetRootComponent(InfluenceRange);
    InfluenceRange->SetSphereRadius(500.f);
    InfluenceRange->SetCollisionResponseToAllChannels(ECR_Ignore);
    InfluenceRange->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    InfluenceRange->OnComponentBeginOverlap.AddDynamic(this, &AFlag::OnInfluencerEnter);
    InfluenceRange->OnComponentEndOverlap.AddDynamic(this, &AFlag::OnInfluencerExit);
}

void AFlag::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AFlag, CapturePercent);
    DOREPLIFETIME(AFlag, Ownership);
    DOREPLIFETIME(AFlag, bIsCaptured);
    DOREPLIFETIME(AFlag, ZoneID);
    DOREPLIFETIME(AFlag, TeamOneCaptureRate);
    DOREPLIFETIME(AFlag, TeamTwoCaptureRate);
    DOREPLIFETIME(AFlag, bCaptureEnabled);
    DOREPLIFETIME(AFlag, BannerState);
}

float AFlag::GetInfluenceRadius() const
{
    return InfluenceRange ? InfluenceRange->GetScaledSphereRadius() : 0.f;
}

void AFlag::SetCaptureEnabled(bool bEnabled)
{
    bCaptureEnabled = bEnabled;
}

void AFlag::OnRep_BannerState()
{
    ApplyBannerState();
}

void AFlag::ApplyBannerState()
{
    if (!BannerActor) return;
    UNiagaraComponent* NC = BannerActor->GetNiagaraComponent();
    if (!NC) return;

    if (BannerState == 0)
    {
        NC->SetPaused(false);
        NC->DeactivateImmediate();
        GetWorldTimerManager().ClearTimer(BannerPauseTimerHandle);
    }
    else
    {
        FLinearColor Color = NeutralBannerColor;
        if (BannerState == 2) Color = TeamOneBannerColor;
        else if (BannerState == 3) Color = TeamTwoBannerColor;

        // Kill any existing banner instantly
        NC->SetPaused(false);
        NC->DeactivateImmediate();

        // Start fresh with new color
        NC->SetVariableLinearColor(FName("Colour"), Color);
        NC->Activate(true);

        if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
        {
            Audio->PlayAtLocation(ChrisGameplayTags::Audio_World_FlagSmash, GetActorLocation());
        }

        // After 4 seconds (slam done), freeze in place
        GetWorldTimerManager().SetTimer(BannerPauseTimerHandle, [NC]()
            {
                if (NC)
                {
                    NC->SetPaused(true);
                }
            }, 4.0f, false);
    }
}

void AFlag::PlayBannerSlam(FLinearColor Color)
{
    if (Color == TeamOneBannerColor)
        BannerState = 2;
    else if (Color == TeamTwoBannerColor)
        BannerState = 3;
    else
        BannerState = 1;

    // OnRep doesn't fire on server, so apply locally too
    ApplyBannerState();
}

void AFlag::DismissBanner()
{
    BannerState = 0;
    ApplyBannerState();
}


void AFlag::PlayTeamCaptureBanner(EFlagOwnership Team)
{
    if (!BannerActor) return;

    UNiagaraComponent* NC = BannerActor->GetNiagaraComponent();
    if (!NC) return;

    // Unpause — lets the disappear animation play out naturally
    NC->SetPaused(false);
    GetWorldTimerManager().ClearTimer(BannerPauseTimerHandle);

    // After the disappear animation finishes, slam the team color version
    FLinearColor TeamColor = (Team == EFlagOwnership::TeamOne) ? TeamOneBannerColor : TeamTwoBannerColor;

    GetWorldTimerManager().SetTimer(BannerTransitionTimerHandle, [this, TeamColor]()
        {
            PlayBannerSlam(TeamColor);
        }, 1.0f, false);
}

void AFlag::ResetCapture()
{
    CapturePercent = 0.f;
    Ownership = EFlagOwnership::Neutral;
    bIsCaptured = false;
    TeamOneCaptureRate = 0.f;
    TeamTwoCaptureRate = 0.f;
    OverlappingHeroes.Empty();
    OverlappingInfluencers.Empty();
    DismissBanner();
}

void AFlag::BeginPlay()
{
    Super::BeginPlay();

}

void AFlag::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!HasAuthority() || bIsCaptured || !bCaptureEnabled)
        return;

    RecalculateCaptureRates();

    // Zone time and XP accrue for anyone standing here while the flag is uncaptured — including a contested zone where influence cancels out.
    AChrisGameState* GS = GetWorld()->GetGameState<AChrisGameState>();
    for (AChrisPlayerCharacter* Hero : OverlappingHeroes)
    {
        if (!Hero || Hero->IsDead()) continue;

        Hero->ZoneTimeAccumulator += DeltaTime;

        // Total capture duration for match stats — never consumed
        if (GS) { GS->AddCaptureTime(Hero, DeltaTime); }

        if (Hero->ZoneTimeAccumulator >= ZoneTimeThreshold)
        {
            Hero->ZoneTimeAccumulator -= ZoneTimeThreshold;
            if (UAbilitySystemComponent* ASC = Hero->GetAbilitySystemComponent())
            {
                ASC->ApplyModToAttribute(UCHeroAttributeSet::GetExperienceAttribute(), EGameplayModOp::Additive, ZoneXPReward);
            }
        }
    }

    // Calculate net capture rate: positive = TeamOne gaining, negative = TeamTwo gaining
    float NetRate = TeamOneCaptureRate - TeamTwoCaptureRate;

    // No one in the zone or perfectly balanced — nothing happens
    if (FMath::IsNearlyZero(NetRate))
        return;

    // Determine which team the net rate favours
    EFlagOwnership FavouringTeam = (NetRate > 0.f) ? EFlagOwnership::TeamOne : EFlagOwnership::TeamTwo;
    float AbsRate = FMath::Abs(NetRate);

    if (Ownership == EFlagOwnership::Neutral || Ownership == FavouringTeam)
    {
        // Same team as current ownership (or neutral) — increase capture
        Ownership = FavouringTeam;
        CapturePercent += AbsRate * DeltaTime;
    }
    else
    {
        // Opposing team — drain current capture first
        CapturePercent -= AbsRate * DeltaTime;

        if (CapturePercent <= 0.f)
        {
            // Crossed zero — flip ownership, carry over the remainder
            CapturePercent = FMath::Abs(CapturePercent);
            Ownership = FavouringTeam;
        }
    }

    // Clamp and check for capture
    CapturePercent = FMath::Clamp(CapturePercent, 0.f, 100.f);

    if (CapturePercent >= 100.f)
    {
        bIsCaptured = true;
        UE_LOG(LogTemp, Warning, TEXT("FLAG CAPTURED by %s!"), Ownership == EFlagOwnership::TeamOne ? TEXT("Team One (Red)") : TEXT("Team Two (Blue)"));
        OnFlagCaptured.Broadcast(Ownership);

        // Dismiss white banner and slam team color
        PlayTeamCaptureBanner(Ownership);
    }

}

float AFlag::GetActorCaptureWeight(AActor* Actor) const
{
    if (!Actor)
        return 0.f;

    // Hero = HeroCaptureWeight (3) + DominionBonus from skill
    if (AChrisPlayerCharacter* Hero = Cast<AChrisPlayerCharacter>(Actor))
    {
        float Weight = HeroCaptureWeight;

        // Check for Dominion skill — adds extra influence points
        UAbilitySystemComponent* ASC = Hero->GetAbilitySystemComponent();
        if (ASC)
        {
            bool bFound = false;
            float Bonus = ASC->GetGameplayAttributeValue(UChrisAttributeSet::GetDominionBonusAttribute(), bFound);
            if (bFound && Bonus > 0.f)
            {
                Weight += Bonus;
            }
        }

        return Weight;
    }

    // If it implements the team interface but isn't a hero, it's an AI minion
    if (Cast<IGenericTeamAgentInterface>(Actor))
        return MinionCaptureWeight;

    return 0.f;
}

void AFlag::RecalculateCaptureRates()
{
    const float PreviousTeamOne = TeamOneCaptureRate;
    const float PreviousTeamTwo = TeamTwoCaptureRate;

    TeamOneCaptureRate = 0.f;
    TeamTwoCaptureRate = 0.f;

    for (auto It = OverlappingInfluencers.CreateIterator(); It; ++It)
    {
        AActor* Influencer = *It;

        // Destroyed without an exit event — drop it from the set
        if (!IsValid(Influencer))
        {
            It.RemoveCurrent();
            continue;
        }

        // Dead bodies stay overlapping, but they shouldn't hold the zone
        if (const AChrisCharacter* Character = Cast<AChrisCharacter>(Influencer))
        {
            if (Character->IsDead()) { continue; }
        }

        const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(Influencer);
        if (!TeamAgent) { continue; }

        const float Weight = GetActorCaptureWeight(Influencer);

        if (TeamAgent->GetGenericTeamId().GetId() == 0)
        {
            TeamOneCaptureRate += Weight;
        }
        else if (TeamAgent->GetGenericTeamId().GetId() == 1)
        {
            TeamTwoCaptureRate += Weight;
        }
    }

    // Only tell the UI when something actually moved
    if (!FMath::IsNearlyEqual(PreviousTeamOne, TeamOneCaptureRate)
        || !FMath::IsNearlyEqual(PreviousTeamTwo, TeamTwoCaptureRate))
    {
        OnTeamInfluenceUpdated.Broadcast(TeamOneCaptureRate, TeamTwoCaptureRate);
    }
}

void AFlag::OnInfluencerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (bIsCaptured)
        return;

    if (!Cast<IGenericTeamAgentInterface>(OtherActor))
        return;

    // Only track membership
    OverlappingInfluencers.Add(OtherActor);

    if (AChrisPlayerCharacter* Hero = Cast<AChrisPlayerCharacter>(OtherActor))
    {
        OverlappingHeroes.Add(Hero);
    }
}

void AFlag::OnInfluencerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (bIsCaptured)
        return;

    OverlappingInfluencers.Remove(OtherActor);

    if (AChrisPlayerCharacter* Hero = Cast<AChrisPlayerCharacter>(OtherActor))
    {
        OverlappingHeroes.Remove(Hero);
    }
}
