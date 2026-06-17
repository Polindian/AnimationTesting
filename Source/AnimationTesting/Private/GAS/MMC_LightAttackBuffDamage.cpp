// Christopher Naglik All Rights Reserved

#include "GAS/MMC_LightAttackBuffDamage.h"
#include "GAS/ChrisAttributeSet.h"

UMMC_LightAttackBuffDamage::UMMC_LightAttackBuffDamage()
{
    // Capture AttackDamage from the source (the attacker)
    DamageCaptureDefinition.AttributeToCapture = UChrisAttributeSet::GetAttackDamageAttribute();
    DamageCaptureDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;
    DamageCaptureDefinition.bSnapshot = false;
    RelevantAttributesToCapture.Add(DamageCaptureDefinition);

    // Capture Armour from the target (the defender)
    ArmourCaptureDefinition.AttributeToCapture = UChrisAttributeSet::GetArmourAttribute();
    ArmourCaptureDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
    ArmourCaptureDefinition.bSnapshot = false;
    RelevantAttributesToCapture.Add(ArmourCaptureDefinition);

    // Capture LightAttackBuffDamage from the source (the attacker)
    LightAttackBonusCaptureDefinition.AttributeToCapture = UChrisAttributeSet::GetLightAttackBuffDamageAttribute();
    LightAttackBonusCaptureDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;
    LightAttackBonusCaptureDefinition.bSnapshot = false;
    RelevantAttributesToCapture.Add(LightAttackBonusCaptureDefinition);
}

float UMMC_LightAttackBuffDamage::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    FAggregatorEvaluateParameters EvaluationParameters;
    EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

    float AttackDamage = 0.f;
    GetCapturedAttributeMagnitude(DamageCaptureDefinition, Spec, EvaluationParameters, AttackDamage);

    float Armour = 0.f;
    GetCapturedAttributeMagnitude(ArmourCaptureDefinition, Spec, EvaluationParameters, Armour);

    float LightAttackBonus = 0.f;
    GetCapturedAttributeMagnitude(LightAttackBonusCaptureDefinition, Spec, EvaluationParameters, LightAttackBonus);

    // Damage = AttackDamage * (1 + LightAttackBonus) * ArmourReduction
    // LightAttackBonus is 0.0 by default, so this equals MMC_BaseAttackDamage when no buff is active
    // Bladed Edge sets LightAttackBonus to 0.1 for a 10% increase
    float Damage = AttackDamage * (1.f + LightAttackBonus) * (1.f - Armour / (Armour + 100.f));
    return -Damage;
}