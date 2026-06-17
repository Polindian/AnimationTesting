// Christopher Naglik All Rights Reserved

#include "GAS/MMC_HeavyAttackBuffDamage.h"
#include "GAS/ChrisAttributeSet.h"

UMMC_HeavyAttackBuffDamage::UMMC_HeavyAttackBuffDamage()
{
    // Capture HeavyAttackBuffDamage from the source (the attacker)
    HeavyAttackBonusCaptureDefinition.AttributeToCapture = UChrisAttributeSet::GetHeavyAttackBuffDamageAttribute();
    HeavyAttackBonusCaptureDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;
    HeavyAttackBonusCaptureDefinition.bSnapshot = false;
    RelevantAttributesToCapture.Add(HeavyAttackBonusCaptureDefinition);
}

float UMMC_HeavyAttackBuffDamage::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    FAggregatorEvaluateParameters EvaluationParameters;
    EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

    float HeavyAttackBonus = 0.f;
    GetCapturedAttributeMagnitude(HeavyAttackBonusCaptureDefinition, Spec, EvaluationParameters, HeavyAttackBonus);

    // Returns 1.0 with no buff, 1.2 with Juggernaut (20% increase)
    // The curve table damage value goes in the Coefficient field of the GE
    // Final = Coefficient * (1 + Bonus) = -60 * 1.2 = -72
    return (1.f + HeavyAttackBonus);
}