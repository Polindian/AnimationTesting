// Christopher Naglik All Rights Reserved

#include "GAS/GAP_Dead.h"
#include "GAS/ChrisAbilitySystemStatics.h"
#include "GAS/ChrisAttributeSet.h"
#include "GAS/CHeroAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemInterface.h"
#include "Framework/ChrisGameMode.h"
#include "Engine/OverlapResult.h"

UGAP_Dead::UGAP_Dead()
{
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

    FAbilityTriggerData TriggerData;
    TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
    TriggerData.TriggerTag = UChrisAbilitySystemStatics::GetDeadStatsTag();
    AbilityTriggers.Add(TriggerData);

    ActivationBlockedTags.RemoveTag(UChrisAbilitySystemStatics::GetStunStatsTag());
}

void UGAP_Dead::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (K2_HasAuthority())
    {
        AActor* Killer = TriggerEventData->ContextHandle.GetEffectCauser();
        if (!Killer || !UChrisAbilitySystemStatics::IsHero(Killer))
        {
            Killer = nullptr;
        }

        // No killer identified — no rewards
        if (!Killer)
        {
            K2_EndAbility();
            return;
        }

     
        float TotalExperienceReward = UChrisAbilitySystemStatics::IsHero(GetAvatarActorFromActorInfo()) ? HeroExperienceReward : BaseExperenceReward;
        float TotalSoulReward = UChrisAbilitySystemStatics::IsHero(GetAvatarActorFromActorInfo()) ? HeroSoulReward : BaseSoulReward;

        // === Stacked Odds: 15% chance to double total rewards BEFORE distribution ===
        IAbilitySystemInterface* KillerASI = Cast<IAbilitySystemInterface>(Killer);
        if (KillerASI && KillerASI->GetAbilitySystemComponent())
        {
            float StackedOddsChance = 0.f;
            bool bStackedFound = false;
            StackedOddsChance = KillerASI->GetAbilitySystemComponent()->GetGameplayAttributeValue(UChrisAttributeSet::GetStackedOddsChanceAttribute(), bStackedFound);

            if (bStackedFound && StackedOddsChance > 0.f && FMath::FRand() < StackedOddsChance)
            {
                TotalExperienceReward *= 2.f;
                TotalSoulReward *= 2.f;
                UE_LOG(LogTemp, Warning, TEXT("[StackedOdds] DOUBLED! XP: %.0f, Soul: %.0f"), TotalExperienceReward, TotalSoulReward);
            }
        }

        // === Find friendly allies near the Killer (within AllyShareRadius, excluding Killer) ===
        TArray<AActor*> NearbyAllies = GetFriendlyAlliesNearKiller(Killer);

        if (NearbyAllies.Num() == 0)
        {
            // No allies nearby — Killer gets 100% of rewards
            float KillerXP = FMath::RoundToFloat(TotalExperienceReward);
            float KillerSoul = FMath::RoundToFloat(TotalSoulReward);

            FGameplayEffectSpecHandle KillerSpec = MakeOutgoingGameplayEffectSpec(RewardEffect);
            KillerSpec.Data->SetSetByCallerMagnitude(UChrisAbilitySystemStatics::GetExperienceAttributeTag(), KillerXP);
            KillerSpec.Data->SetSetByCallerMagnitude(UChrisAbilitySystemStatics::GetSoulAttributeTag(), KillerSoul);
            K2_ApplyGameplayEffectSpecToTarget(KillerSpec, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(Killer));

            UE_LOG(LogTemp, Log, TEXT("[Rewards] Killer gets 100%%: XP=%.0f Soul=%.0f"), KillerXP, KillerSoul);
        }
        else
        {
            // Allies nearby — Killer gets 50%, allies split remaining 50%
            float KillerXP = FMath::RoundToFloat(TotalExperienceReward * 0.5f);
            float KillerSoul = FMath::RoundToFloat(TotalSoulReward * 0.5f);

            FGameplayEffectSpecHandle KillerSpec = MakeOutgoingGameplayEffectSpec(RewardEffect);
            KillerSpec.Data->SetSetByCallerMagnitude(UChrisAbilitySystemStatics::GetExperienceAttributeTag(), KillerXP);
            KillerSpec.Data->SetSetByCallerMagnitude(UChrisAbilitySystemStatics::GetSoulAttributeTag(), KillerSoul);
            K2_ApplyGameplayEffectSpecToTarget(KillerSpec, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(Killer));

            UE_LOG(LogTemp, Log, TEXT("[Rewards] Killer gets 50%%: XP=%.0f Soul=%.0f"), KillerXP, KillerSoul);

            // Split remaining 50% evenly among allies, rounded to nearest whole number
            float AllyXP = FMath::RoundToFloat((TotalExperienceReward * 0.5f) / NearbyAllies.Num());
            float AllySoul = FMath::RoundToFloat((TotalSoulReward * 0.5f) / NearbyAllies.Num());

            FGameplayEffectSpecHandle AllySpec = MakeOutgoingGameplayEffectSpec(RewardEffect);
            AllySpec.Data->SetSetByCallerMagnitude(UChrisAbilitySystemStatics::GetExperienceAttributeTag(), AllyXP);
            AllySpec.Data->SetSetByCallerMagnitude(UChrisAbilitySystemStatics::GetSoulAttributeTag(), AllySoul);
            K2_ApplyGameplayEffectSpecToTarget(AllySpec, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActorArray(NearbyAllies, true));

            UE_LOG(LogTemp, Log, TEXT("[Rewards] %d allies each get: XP=%.0f Soul=%.0f"), NearbyAllies.Num(), AllyXP, AllySoul);
        }

        // Report kill to GameMode for round-end scoring
        if (AChrisGameMode* GM = Cast<AChrisGameMode>(GetWorld()->GetAuthGameMode()))
        {
            IGenericTeamAgentInterface* KillerTeamAgent = Cast<IGenericTeamAgentInterface>(Killer);
            if (!KillerTeamAgent)
            {
                if (APawn* KillerPawn = Cast<APawn>(Killer))
                {
                    KillerTeamAgent = Cast<IGenericTeamAgentInterface>(KillerPawn->GetController());
                }
            }
            if (KillerTeamAgent)
            {
                GM->ReportKill(KillerTeamAgent->GetGenericTeamId());
            }
        }

        K2_EndAbility();
    }
}

// Finds all friendly hero allies near the Killer within AllyShareRadius (excludes the Killer)
TArray<AActor*> UGAP_Dead::GetFriendlyAlliesNearKiller(AActor* Killer) const
{
    TArray<AActor*> Allies;

    if (!Killer || !GetWorld())
    {
        return Allies;
    }

    FCollisionObjectQueryParams ObjectQueryParams;
    ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
    FCollisionShape CollisionShape;
    CollisionShape.SetSphere(AllyShareRadius);

    TArray<FOverlapResult> OverlapResults;
    if (GetWorld()->OverlapMultiByObjectType(OverlapResults, Killer->GetActorLocation(), FQuat::Identity, ObjectQueryParams, CollisionShape))
    {
        for (const FOverlapResult& OverlapResult : OverlapResults)
        {
            AActor* OtherActor = OverlapResult.GetActor();

            // Skip self
            if (OtherActor == Killer) continue;

            // Must be a hero
            if (!UChrisAbilitySystemStatics::IsHero(OtherActor)) continue;

            // Must be on the same team as the Killer (Friendly attitude)
            const IGenericTeamAgentInterface* OtherTeamInterface = Cast<IGenericTeamAgentInterface>(OtherActor);
            if (!OtherTeamInterface) continue;

            if (OtherTeamInterface->GetTeamAttitudeTowards(*Killer) != ETeamAttitude::Friendly)
            {
                continue;
            }

            Allies.Add(OtherActor);
        }
    }

    return Allies;
}