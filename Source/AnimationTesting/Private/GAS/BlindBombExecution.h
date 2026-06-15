// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "BlindBombExecution.generated.h"

/**
 * Custom execution calculation for the Nightflare blind bomb consumable.
 *
 * When the consumed GE fires on the caster, this class:
 *   1. Does a sphere overlap around the caster (BlindRadius)
 *   2. Finds all hostile pawns in range
 *   3. Applies GE_Blinded to each of them
 *
 * This lets us keep GA_GrantAbility on the data asset for cooldown
 * handling while still doing AoE logic through the Consumed Effect slot.
 */
UCLASS()
class UBlindBombExecution : public UGameplayEffectExecutionCalculation
{
    GENERATED_BODY()

public:
    virtual void Execute_Implementation(
        const FGameplayEffectCustomExecutionParameters& ExecutionParams,
        FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

private:
    // The GE to apply to each blinded enemy.
    // We load this by class path so it doesn't need to be a UPROPERTY
    // on the execution calc — execution calcs are CDOs (class default objects)
    // and don't support per-instance editor properties.
    // Instead we'll reference it via a soft class path loaded at runtime.
};