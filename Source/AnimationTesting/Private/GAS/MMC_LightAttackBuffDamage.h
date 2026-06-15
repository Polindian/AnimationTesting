// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_LightAttackBuffDamage.generated.h"

UCLASS()
class UMMC_LightAttackBuffDamage : public UGameplayModMagnitudeCalculation
{
    GENERATED_BODY()
public:
    UMMC_LightAttackBuffDamage();

    virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
    FGameplayEffectAttributeCaptureDefinition DamageCaptureDefinition;
    FGameplayEffectAttributeCaptureDefinition ArmourCaptureDefinition;
    FGameplayEffectAttributeCaptureDefinition LightAttackBonusCaptureDefinition;
};