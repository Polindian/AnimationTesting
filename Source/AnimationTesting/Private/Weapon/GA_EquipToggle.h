// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GAS/ChrisGameplayAbility.h"
#include "GA_EquipToggle.generated.h"

/**
 * 
 */
UCLASS()
class UGA_EquipToggle : public UChrisGameplayAbility
{
	GENERATED_BODY()
	
public:
    UGA_EquipToggle();

    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility, bool bWasCancelled) override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* EquipMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* UnequipMontage;
};
