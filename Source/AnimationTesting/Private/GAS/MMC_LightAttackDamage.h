// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_LightAttackDamage.generated.h"

/**
 * 
 */
UCLASS()
class UMMC_LightAttackDamage : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
    UMMC_LightAttackDamage();
    virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
    FGameplayEffectAttributeCaptureDefinition DamageCaptureDefinition;
    FGameplayEffectAttributeCaptureDefinition ArmourCaptureDefinition;

    // === NEW === captures the light attack bonus from source
    FGameplayEffectAttributeCaptureDefinition LightAttackBonusCaptureDefinition;
};
