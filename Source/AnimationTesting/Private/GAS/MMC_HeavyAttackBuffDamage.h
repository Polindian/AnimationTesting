// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_HeavyAttackBuffDamage.generated.h"

/**
 * 
 */
UCLASS()
class UMMC_HeavyAttackBuffDamage : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
    UMMC_HeavyAttackBuffDamage();
    virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
    FGameplayEffectAttributeCaptureDefinition HeavyAttackBonusCaptureDefinition;
};
