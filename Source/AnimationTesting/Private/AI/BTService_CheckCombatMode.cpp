// Christopher Naglik All Rights Reserved

#include "AI/BTService_CheckCombatMode.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/ChrisAbilitySystemStatics.h"
#include "GAS/ChrisAttributeSet.h"
#include "GAS/CHeroAttributeSet.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Player/ChrisPlayerCharacter.h"
#include "Framework/Flag.h"
#include "EngineUtils.h"

UBTService_CheckCombatMode::UBTService_CheckCombatMode()
{
    NodeName = "Check Combat Mode";

    // Ticks every 0.3s — fast enough for combat decisions and AoE detection
    Interval = 0.3f;
    RandomDeviation = 0.05f;

    // ======================================================
    // BUILD THE HEAVY ATTACK TAG CONTAINER
    // ======================================================
    HeavyAttackTags.AddTag(FGameplayTag::RequestGameplayTag("ability.heavyattack"));
    HeavyAttackTags.AddTag(FGameplayTag::RequestGameplayTag("ability.heavyattack1"));
}

void UBTService_CheckCombatMode::CacheFlags(UWorld* World)
{
    if (!World) return;

    AllFlags.Empty();
    for (TActorIterator<AFlag> It(World); It; ++It)
    {
        AllFlags.Add(*It);
    }

    bFlagsCached = (AllFlags.Num() > 0);
}

// ======================================================
// CHECK FOR AOE DANGER
//
// Scans all perceived enemies for active heavy attack tags.
// If found, checks whether THIS skeleton is within the
// ability's danger radius
// ======================================================
bool UBTService_CheckCombatMode::CheckForAoEDanger(UAIPerceptionComponent* PerceptionComp, const FVector& PawnLocation, FVector& OutDodgeLocation) const
{
    TArray<AActor*> PerceivedEnemies;
    PerceptionComp->GetPerceivedHostileActors(PerceivedEnemies);

    for (AActor* Enemy : PerceivedEnemies)
    {
        if (!Enemy || !IsValid(Enemy)) continue;

        UAbilitySystemComponent* EnemyASC =
            UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Enemy);
        if (!EnemyASC) continue;

        // Check if this enemy is CURRENTLY EXECUTING a heavy attack.
        bool bHeavyAttackActive = false;
        const TArray<FGameplayAbilitySpec>& Specs = EnemyASC->GetActivatableAbilities();
        for (const FGameplayAbilitySpec& Spec : Specs)
        {
            if (Spec.IsActive() && Spec.Ability)
            {
                if (Spec.Ability->AbilityTags.HasAny(HeavyAttackTags))
                {
                    bHeavyAttackActive = true;
                    break;
                }
            }
        }

        if (!bHeavyAttackActive)
        {
            continue;
        }

        // Enemy IS doing a heavy attack — determine danger radius
        bool bFoundSmash, bFoundStun;
        float SmashRadius = EnemyASC->GetGameplayAttributeValue(
            UChrisAttributeSet::GetSmashSweepRadiusAttribute(), bFoundSmash);
        float StunRadius = EnemyASC->GetGameplayAttributeValue(
            UChrisAttributeSet::GetStunSweepRadiusAttribute(), bFoundStun);

        float DangerRadius = FMath::Max(SmashRadius, StunRadius);
        if (DangerRadius <= 0.f)
        {
            DangerRadius = DefaultDangerRadius;
        }

        // Two zones:
        //   INNER (within DangerRadius): skeleton MUST flee outward
        //   BUFFER (DangerRadius to DangerRadius+Buffer): skeleton STOPS, doesn't advance
        FVector EnemyLocation = Enemy->GetActorLocation();
        float DistToEnemy = FVector::Dist(PawnLocation, EnemyLocation);
        float TotalDangerZone = DangerRadius + DodgeBufferDistance;

        if (DistToEnemy <= TotalDangerZone)
        {
            FVector EscapeDir;

            if (DistToEnemy > 10.f)
            {
                EscapeDir = (PawnLocation - EnemyLocation).GetSafeNormal();
            }
            else
            {
                EscapeDir = FVector(FMath::RandRange(-1.f, 1.f),
                    FMath::RandRange(-1.f, 1.f), 0.f).GetSafeNormal();
            }

            EscapeDir.Z = 0.f;
            EscapeDir = EscapeDir.GetSafeNormal();

            // All skeletons flee to the same point: past the total danger zone
            float FleeDistance = TotalDangerZone + DodgeExtraDistance;
            OutDodgeLocation = EnemyLocation + (EscapeDir * FleeDistance);

            return true;
        }
    }

    return false;
}
// ======================================================
// SCORE TARGET
//
// Assigns a priority score to a potential target.
// Higher score = more desirable target.
//
// Factors:
//   Distance:        closer = more score (linear 0-500)
//   Hero bonus:      +200 if target is a player
//   Capturing flag:  +400 if target is inside a zone
//   Low health:      +300 if target is below 25% HP
//   Level:           +80 per hero level
// ======================================================
float UBTService_CheckCombatMode::ScoreTarget(AActor* Enemy, const FVector& PawnLocation, uint8 MyTeamId) const
{
    float Score = 0.f;

    // --- DISTANCE ---
    float Dist = FVector::Dist(PawnLocation, Enemy->GetActorLocation());
    float DistScore = FMath::Clamp(1.f - (Dist / 1000.f), 0.f, 1.f) * DistanceMaxScore;
    Score += DistScore;

    // --- HERO BONUS ---
    bool bIsHero = (Cast<AChrisPlayerCharacter>(Enemy) != nullptr);
    if (bIsHero)
    {
        Score += HeroBonus;
    }

    // --- CAPTURING FLAG BONUS ---
    FVector EnemyLoc = Enemy->GetActorLocation();
    for (AFlag* Flag : AllFlags)
    {
        if (!Flag || !IsValid(Flag) || Flag->IsCaptured()) continue;

        float DistToFlag = FVector::Dist(EnemyLoc, Flag->GetActorLocation());
        if (DistToFlag <= Flag->GetInfluenceRadius())
        {
            Score += CapturingFlagBonus;
            break;
        }
    }

    // --- LOW HEALTH + LEVEL ---
    UAbilitySystemComponent* EnemyASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Enemy);
    if (EnemyASC)
    {
        bool bFoundHP, bFoundMaxHP;
        float HP = EnemyASC->GetGameplayAttributeValue(UChrisAttributeSet::GetHealthAttribute(), bFoundHP);
        float MaxHP = EnemyASC->GetGameplayAttributeValue(UChrisAttributeSet::GetMaxHealthAttribute(), bFoundMaxHP);

        if (bFoundHP && bFoundMaxHP && MaxHP > 0.f && (HP / MaxHP) < 0.25f)
        {
            Score += LowHealthBonus;
        }

        if (bIsHero)
        {
            bool bFoundLevel;
            float Level = EnemyASC->GetGameplayAttributeValue(
                UCHeroAttributeSet::GetLevelAttribute(), bFoundLevel);
            if (bFoundLevel)
            {
                Score += Level * LevelWeight;
            }
        }
    }

    return Score;
}

void UBTService_CheckCombatMode::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    // ======================================================
    // SETUP
    // ======================================================
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!AIC) return;

    APawn* Pawn = AIC->GetPawn();
    if (!Pawn) return;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return;

    if (!bFlagsCached)
    {
        CacheFlags(AIC->GetWorld());
        if (!bFlagsCached) return;
    }

    uint8 MyTeamId = AIC->GetGenericTeamId().GetId();
    FVector PawnLocation = Pawn->GetActorLocation();

    UAIPerceptionComponent* PerceptionComp = AIC->FindComponentByClass<UAIPerceptionComponent>();
    if (!PerceptionComp) return;

    // ======================================================
    // AOE DODGE CHECK (runs FIRST — highest priority)
    // When the danger passes (enemy finishes attack or we
    // moved out of range), bMustDodge = false, and the AI
    // falls back to normal branches.
    // ======================================================
    FVector DodgeLocation;
    bool bDangerDetected = CheckForAoEDanger(PerceptionComp, PawnLocation, DodgeLocation);

    if (bDangerDetected)
    {
        BB->SetValueAsBool(MustDodgeKeyName, true);
        BB->SetValueAsVector(DodgeLocationKeyName, DodgeLocation);
    }
    else
    {
        BB->SetValueAsBool(MustDodgeKeyName, false);
    }

   

    // ======================================================
    // ZONE CONTROL: Should the AI chase or stay stationary?
    // ======================================================
    bool bShouldMove = true;

    for (AFlag* Flag : AllFlags)
    {
        if (!Flag || !IsValid(Flag) || Flag->IsCaptured()) continue;

        float DistToFlag = FVector::Dist(PawnLocation, Flag->GetActorLocation());
        float FlagRadius = Flag->GetInfluenceRadius();

        if (DistToFlag <= FlagRadius)
        {
            float MyRate = (MyTeamId == 0) ? Flag->GetTeamOneCaptureRate() : Flag->GetTeamTwoCaptureRate();
            float EnemyRate = (MyTeamId == 0) ? Flag->GetTeamTwoCaptureRate() : Flag->GetTeamOneCaptureRate();
            float Advantage = MyRate - EnemyRate;

            if (Advantage >= RateAdvantageThreshold)
            {
                AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetKeyName));
                if (Target && IsValid(Target))
                {
                    float TargetDistToFlag = FVector::Dist(Target->GetActorLocation(), Flag->GetActorLocation());
                    if (TargetDistToFlag > FlagRadius)
                    {
                        bShouldMove = false;
                    }
                }
            }

            break;
        }
    }

    BB->SetValueAsBool(ShouldMoveKeyName, bShouldMove);

    // ======================================================
    // TARGET MANAGEMENT
    // ======================================================
    AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(TargetKeyName));

    // Validate current target every tick
    bool bKeepCurrentTarget = false;
    if (CurrentTarget && IsValid(CurrentTarget))
    {
        float DistToTarget = FVector::Dist(PawnLocation, CurrentTarget->GetActorLocation());
        if (DistToTarget <= 1000.f)
        {
            UAbilitySystemComponent* TargetASC =
                UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CurrentTarget);
            bool bTargetDead = TargetASC && TargetASC->HasMatchingGameplayTag(
                UChrisAbilitySystemStatics::GetDeadStatsTag());

            if (!bTargetDead)
            {
                bKeepCurrentTarget = true;
            }
        }
    }

    // If current target is gone/dead — find new one with full scoring
    if (!bKeepCurrentTarget)
    {
        TArray<AActor*> PerceivedEnemies;
        PerceptionComp->GetPerceivedHostileActors(PerceivedEnemies);

        AActor* BestTarget = nullptr;
        float BestScore = -1.f;
        for (AActor* Enemy : PerceivedEnemies)
        {
            if (!Enemy || !IsValid(Enemy)) continue;

            UAbilitySystemComponent* EnemyASC =
                UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Enemy);
            if (EnemyASC && EnemyASC->HasMatchingGameplayTag(
                UChrisAbilitySystemStatics::GetDeadStatsTag()))
            {
                continue;
            }

            float EnemyScore = ScoreTarget(Enemy, PawnLocation, MyTeamId);
            if (EnemyScore > BestScore)
            {
                BestScore = EnemyScore;
                BestTarget = Enemy;
            }
        }

        if (BestTarget)
        {
            BB->SetValueAsObject(TargetKeyName, BestTarget);
            AIC->SetFocus(BestTarget);
        }
        else
        {
            BB->ClearValue(TargetKeyName);
            AIC->ClearFocus(EAIFocusPriority::Gameplay);
        }

        TargetReevalTimer = 0.f;
    }
    else
    {
        // Periodic re-evaluation: check if a better target exists
        TargetReevalTimer += DeltaSeconds;

        if (TargetReevalTimer >= TargetReevalInterval)
        {
            TargetReevalTimer = 0.f;

            TArray<AActor*> PerceivedEnemies;
            PerceptionComp->GetPerceivedHostileActors(PerceivedEnemies);

            AActor* BestTarget = nullptr;
            float BestScore = -1.f;
            for (AActor* Enemy : PerceivedEnemies)
            {
                if (!Enemy || !IsValid(Enemy)) continue;

                UAbilitySystemComponent* EnemyASC =
                    UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Enemy);
                if (EnemyASC && EnemyASC->HasMatchingGameplayTag(
                    UChrisAbilitySystemStatics::GetDeadStatsTag()))
                {
                    continue;
                }

                float EnemyScore = ScoreTarget(Enemy, PawnLocation, MyTeamId);
                if (EnemyScore > BestScore)
                {
                    BestScore = EnemyScore;
                    BestTarget = Enemy;
                }
            }

            // Only switch if significantly better (prevents flickering)
            if (BestTarget && BestTarget != CurrentTarget)
            {
                float CurrentScore = ScoreTarget(CurrentTarget, PawnLocation, MyTeamId);
                if (BestScore > CurrentScore + 150.f)
                {
                    BB->SetValueAsObject(TargetKeyName, BestTarget);
                    AIC->SetFocus(BestTarget);
                }
            }
        }

        // Always maintain focus on current target
        AIC->SetFocus(CurrentTarget);
    }
}