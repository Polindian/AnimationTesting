// Christopher Naglik All Rights Reserved


#include "GAS/GAP_Dead.h"
#include "GAS/ChrisAbilitySystemStatics.h"
#include "GAS/ChrisAttributeSet.h"
#include "GAS/CHeroAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
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

		TArray<AActor*> RewardTargets = GetRewardTargets();

		if (RewardTargets.Num() == 0 && !Killer)
		{
			K2_EndAbility();
			return;
		}

		if(Killer && !RewardTargets.Contains(Killer))
		{
			RewardTargets.Add(Killer);
		}

		bool bFound = false;
		float SelfExperience = GetAbilitySystemComponentFromActorInfo_Ensured()->GetGameplayAttributeValue(UCHeroAttributeSet::GetExperienceAttribute(), bFound);

		float TotalExperienceReward = BaseExperenceReward + ExperienceRewardPerExperience * SelfExperience;
		float TotalSoulReward = BaseSoulReward + SoulRewardPerExperience * SelfExperience;

		if (Killer)
		{
			float KillerExperienceReward = TotalExperienceReward * KillerRewardPortion;
			float KillerSoulReward = TotalSoulReward * KillerRewardPortion;

			FGameplayEffectSpecHandle EffectSpec = MakeOutgoingGameplayEffectSpec(RewardEffect);
			EffectSpec.Data->SetSetByCallerMagnitude(UChrisAbilitySystemStatics::GetExperienceAttributeTag(), KillerExperienceReward);
			EffectSpec.Data->SetSetByCallerMagnitude(UChrisAbilitySystemStatics::GetSoulAttributeTag(), KillerSoulReward);

			K2_ApplyGameplayEffectSpecToTarget(EffectSpec, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(Killer));

			TotalExperienceReward -= KillerExperienceReward;
			TotalSoulReward -= KillerSoulReward;
		}

		float ExperiencePerTarget = TotalExperienceReward / RewardTargets.Num();
		float SoulPerTarget = TotalSoulReward / RewardTargets.Num();

		FGameplayEffectSpecHandle EffectSpec = MakeOutgoingGameplayEffectSpec(RewardEffect);
		EffectSpec.Data->SetSetByCallerMagnitude(UChrisAbilitySystemStatics::GetExperienceAttributeTag(), ExperiencePerTarget);
		EffectSpec.Data->SetSetByCallerMagnitude(UChrisAbilitySystemStatics::GetSoulAttributeTag(), SoulPerTarget);

		K2_ApplyGameplayEffectSpecToTarget(EffectSpec, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActorArray(RewardTargets, true));
		K2_EndAbility();
	}
}

TArray<AActor*> UGAP_Dead::GetRewardTargets() const
{
	TSet<AActor*> OutActors;

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !GetWorld())
	{
		return OutActors.Array();
	}

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(RewardRange);

	TArray<FOverlapResult> OverlapResults;
	if (GetWorld()->OverlapMultiByObjectType(OverlapResults, AvatarActor->GetActorLocation(), FQuat::Identity, ObjectQueryParams, CollisionShape))
	{
		for (const FOverlapResult& OverlapResult : OverlapResults)
		{
			const IGenericTeamAgentInterface* OtherTeamInterface = Cast<IGenericTeamAgentInterface>(OverlapResult.GetActor());

			if(!OtherTeamInterface || OtherTeamInterface->GetTeamAttitudeTowards(*AvatarActor) != ETeamAttitude::Hostile)
			{
				continue;
			}

			if(!UChrisAbilitySystemStatics::IsHero(OverlapResult.GetActor()))
			{
				continue;
			}
			OutActors.Add(OverlapResult.GetActor());
		}
	}
	return OutActors.Array();

}
