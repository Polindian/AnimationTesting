// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GAS/ChrisGameplayAbility.h"
#include "ActiveGameplayEffectHandle.h"
#include "Thrust.generated.h"

UCLASS()
class UThrust : public UChrisGameplayAbility
{
	GENERATED_BODY()
	
public:
	UThrust();
	virtual void  ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;

	static FGameplayTag GetThrustStartTag();

private:
	UPROPERTY(EditDefaultsOnly,Category = "Animation")
	UAnimMontage* ThrustMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Targetting")
	float TargetDetectionRadius = 150.f;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Cue")
	FGameplayTag LocalGameplayCueTag;

	UPROPERTY(EditDefaultsOnly, Category = "Targetting")
	FName TargetActorAttachSocketName = "TargetThrustCenter";

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TSubclassOf<class ATargetActor_Around> TargetActorClass;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	float TargetHitPushSpeed = 2500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> ThrustEffect;

	UPROPERTY()
	class UCharacterMovementComponent* OwnerCharacterMovementComponent;

	UFUNCTION()
	void TargetReceived(const FGameplayAbilityTargetDataHandle& TargetDataHandle);

	UFUNCTION()
	void StartThrust(FGameplayEventData Payload);

	void PushForward();

	FTimerHandle PushForwardInputTimerHandle;

	FActiveGameplayEffectHandle ThrustEffectHandle;
};