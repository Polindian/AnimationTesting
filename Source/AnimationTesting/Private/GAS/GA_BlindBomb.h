// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GAS/ChrisGameplayAbility.h"
#include "GA_BlindBomb.generated.h"


UCLASS()
class UGA_BlindBomb : public UChrisGameplayAbility
{
    GENERATED_BODY()

public:
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

private:
    // How far from the caster to search for enemies (in unreal units).
    UPROPERTY(EditDefaultsOnly, Category = "Blind")
    float BlindRadius = 240.f;

    // The GE applied to each enemy found in range.
    UPROPERTY(EditDefaultsOnly, Category = "Blind")
    TSubclassOf<UGameplayEffect> BlindEffect;
};