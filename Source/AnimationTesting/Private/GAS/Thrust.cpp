// Christopher Naglik All Rights Reserved


#include "GAS/Thrust.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_NetworkSyncPoint.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "GAS/ChrisAbilitySystemStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"
#include "TargetActor_Around.h"

UThrust::UThrust()
{
	ActivationRequiredTags.AddTag(UChrisAbilitySystemStatics::GetWeaponsEquippedTag());
}

void UThrust::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility() || !ThrustMontage)
	{
		K2_EndAbility();
		return;
	}

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayThrustMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ThrustMontage);
		PlayThrustMontage->OnBlendOut.AddDynamic(this, &UThrust::K2_EndAbility);
		PlayThrustMontage->OnCancelled.AddDynamic(this, &UThrust::K2_EndAbility);
		PlayThrustMontage->OnInterrupted.AddDynamic(this, &UThrust::K2_EndAbility);
		PlayThrustMontage->OnCompleted.AddDynamic(this, &UThrust::K2_EndAbility);
		PlayThrustMontage->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitThrustStartEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetThrustStartTag());
		WaitThrustStartEvent->EventReceived.AddDynamic(this, &UThrust::StartThrust);
		WaitThrustStartEvent->ReadyForActivation();
	}
}

void UThrust::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UAbilitySystemComponent* OwnerAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();

	if (OwnerAbilitySystemComponent && ThrustEffectHandle.IsValid())
	{
		OwnerAbilitySystemComponent->RemoveActiveGameplayEffect(ThrustEffectHandle);
	}

	if (PushForwardInputTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(PushForwardInputTimerHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FGameplayTag UThrust::GetThrustStartTag()
{
	return FGameplayTag::RequestGameplayTag("ability.heavyattack2.start");
}


void UThrust::StartThrust(FGameplayEventData Payload)
{
	if (K2_HasAuthority())
	{
		if (ThrustEffect)
		{
			ThrustEffectHandle = BP_ApplyGameplayEffectToOwner(ThrustEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		}
	}

	if (IsLocallyControlled())
	{
		PushForwardInputTimerHandle = GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UThrust::PushForward);
		OwnerCharacterMovementComponent = GetAvatarActorFromActorInfo()->GetComponentByClass<UCharacterMovementComponent>();
	}

	UAbilityTask_WaitTargetData* WaitTargetData = UAbilityTask_WaitTargetData::WaitTargetData(this, NAME_None, EGameplayTargetingConfirmation::CustomMulti, TargetActorClass);
	WaitTargetData->ValidData.AddDynamic(this, &UThrust::TargetReceived);
	WaitTargetData->ReadyForActivation();

	FGenericTeamId OwnerTeamId = FGenericTeamId::NoTeam;
	IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(GetAvatarActorFromActorInfo());

	if (TeamAgent)
	{
		OwnerTeamId = TeamAgent->GetGenericTeamId();
	}

	AGameplayAbilityTargetActor* TargetActor;
	WaitTargetData->BeginSpawningActor(this, TargetActorClass, TargetActor);
	ATargetActor_Around* TargetActorAround = Cast<ATargetActor_Around>(TargetActor);
	if (TargetActorAround)
	{
		TargetActorAround->ConfigureDetection(TargetDetectionRadius, OwnerTeamId, LocalGameplayCueTag);
	}

	WaitTargetData->FinishSpawningActor(this, TargetActor);
	if (TargetActorAround)
	{
		TargetActorAround->AttachToComponent(GetOwningComponentFromActorInfo(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TargetActorAttachSocketName);
	}
	

}

void UThrust::PushForward()
{
	if (OwnerCharacterMovementComponent)
	{
		FVector ForwardActor = GetAvatarActorFromActorInfo()->GetActorForwardVector();
		OwnerCharacterMovementComponent->AddInputVector(ForwardActor);
		PushForwardInputTimerHandle = GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UThrust::PushForward);
	}
}

void UThrust::TargetReceived(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	if (K2_HasAuthority())
	{

		TArray<AActor*> TargetActors = UAbilitySystemBlueprintLibrary::GetAllActorsFromTargetData(TargetDataHandle);
		AActor* OwnerActor = GetAvatarActorFromActorInfo();

		for (AActor* Target : TargetActors)
		{
			if (!Target || !OwnerActor) continue;

			// Push enemy directly via LaunchCharacter
			ACharacter* TargetCharacter = Cast<ACharacter>(Target);
			if (TargetCharacter)
			{
				FVector PushDir = (Target->GetActorLocation() - OwnerActor->GetActorLocation()).GetSafeNormal();
				TargetCharacter->LaunchCharacter(PushDir * TargetHitPushSpeed, true, true);
			}
		}

		// Apply damage after push
		BP_ApplyGameplayEffectToTarget(TargetDataHandle, DamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
	}
}

