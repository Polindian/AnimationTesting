// Christopher Naglik All Rights Reserved


#include "GAS/StabAbility.h"
#include "GAS/GA_Combo.h"
#include "GAS/ChrisAbilitySystemStatics.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameplayTagsManager.h"


UStabAbility::UStabAbility()
{
	ActivationRequiredTags.AddTag(UChrisAbilitySystemStatics::GetWeaponsEquippedTag());
}

void UStabAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, RightStabMontage);
		PlayMontageTask->OnBlendOut.AddDynamic(this, &UStabAbility::K2_EndAbility);
		PlayMontageTask->OnCancelled.AddDynamic(this, &UStabAbility::K2_EndAbility);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UStabAbility::K2_EndAbility);
		PlayMontageTask->OnCompleted.AddDynamic(this, &UStabAbility::K2_EndAbility);
		PlayMontageTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitForEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetRightStabTag());
		WaitForEventTask->EventReceived.AddDynamic(this, &UStabAbility::StartCombo);
		WaitForEventTask->ReadyForActivation();
	}
	NextComboName = NAME_None;
}

FGameplayTag UStabAbility::GetRightStabTag()
{
	return FGameplayTag::RequestGameplayTag("ability.stab.right");
}

const FGenericDamageEffectDefinition* UStabAbility::GetDamageEffectDefinitionForCurrentCombo() const
{
	UAnimInstance* OwnerAnimInstance = GetOwnerAnimInstance();
	if (OwnerAnimInstance)
	{
		FName CurrentComboName = OwnerAnimInstance->Montage_GetCurrentSection(RightStabMontage);
		const FGenericDamageEffectDefinition* EffectDefinition = ComboDamageMap.Find(CurrentComboName);
		return EffectDefinition;
	}
	return nullptr;
}

void UStabAbility::StartCombo(FGameplayEventData EventData)
{
	if (K2_HasAuthority())
	{
		TArray<FHitResult> TargetHitResults = GetHitResultFromSweepLocationTargetData(EventData.TargetData, TargetSweepRadius, ETeamAttitude::Hostile, ShouldDrawDebug());
		for (FHitResult& HitResult : TargetHitResults)
		{
			// Apply push velocity on initial hit
			if (!InitialHitPushVelocity.IsNearlyZero())
			{
				FVector PushVelocity = GetAvatarActorFromActorInfo()->GetActorTransform().TransformVector(InitialHitPushVelocity);
				PushTarget(HitResult.GetActor(), PushVelocity);
			}

			ApplyGameplayEffectToHitResultActor(HitResult, StabDamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		}
	}	

	UAbilityTask_WaitGameplayEvent* WaitComboChangeEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, UGA_Combo::GetComboChangedEventTag(), nullptr, false, false);
	WaitComboChangeEvent->EventReceived.AddDynamic(this, &UStabAbility::HandleComboChangeEvent);
	WaitComboChangeEvent->ReadyForActivation();

	// Collect unique input tags from ComboInputMap and register listeners
	TSet<FGameplayTag> UniqueInputTags;
	for (const auto& Pair : ComboInputMap)
	{
		if (Pair.Value.IsValid())
		{
			UniqueInputTags.Add(Pair.Value);
		}
	}

	for (const FGameplayTag& InputTag : UniqueInputTags)
	{
		UAbilityTask_WaitGameplayEvent* WaitComboCommitEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, InputTag);
		WaitComboCommitEvent->EventReceived.AddDynamic(this, &UStabAbility::HandleComboCommit);
		WaitComboCommitEvent->ReadyForActivation();
	}

	UAbilityTask_WaitGameplayEvent* WaitComboDamageEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, UGA_Combo::GetComboTargetEventTag());
	WaitComboDamageEvent->EventReceived.AddDynamic(this, &UStabAbility::HandleComboDamageEvent);
	WaitComboDamageEvent->ReadyForActivation();
}


	/**********************************************/
	/*                Combo Changes               */
	/**********************************************/

void UStabAbility::HandleComboChangeEvent(FGameplayEventData EventData)
{
	FGameplayTag EventTag = EventData.EventTag;
	if (EventTag == UGA_Combo::GetComboChangedEventEndTag())
	{
		NextComboName = NAME_None;
		return;
	}
	TArray<FName> TagNames;
	UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag, TagNames);

	NextComboName = TagNames.Last();
}

void UStabAbility::HandleComboCommit(FGameplayEventData EventData)
{
	if (NextComboName == NAME_None)
	{
		return;
	}

	UAnimInstance* OwnerAnimInstance = GetOwnerAnimInstance();
	if (!OwnerAnimInstance)
	{
		return;
	}


	// Get current section and check if the received input tag matches what's required
	FName CurrentSection = OwnerAnimInstance->Montage_GetCurrentSection(RightStabMontage);
	const FGameplayTag* RequiredInputTag = ComboInputMap.Find(CurrentSection);

	// Only advance if this is the correct input for the current section
	if (RequiredInputTag && *RequiredInputTag == EventData.EventTag)
	{
		OwnerAnimInstance->Montage_SetNextSection(CurrentSection, NextComboName, RightStabMontage);
	}
}

void UStabAbility::HandleComboDamageEvent(FGameplayEventData EventData)
{
	if (K2_HasAuthority())
	{
		TArray<FHitResult> TargetHitResults = GetHitResultFromSweepLocationTargetData(EventData.TargetData, TargetSweepRadius, ETeamAttitude::Hostile);
		const FGenericDamageEffectDefinition* EffectDefinition = GetDamageEffectDefinitionForCurrentCombo();
		if (!EffectDefinition)
		{
			return;
		}
		for (FHitResult& HitResult : TargetHitResults)
		{
			FVector PushVelocity = GetAvatarActorFromActorInfo()->GetActorTransform().TransformVector(EffectDefinition->PushVelocity);
			PushTarget(HitResult.GetActor(), PushVelocity);
			ApplyGameplayEffectToHitResultActor(HitResult, EffectDefinition->DamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		}
	}
}
