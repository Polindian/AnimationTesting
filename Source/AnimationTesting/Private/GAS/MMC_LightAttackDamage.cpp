// Christopher Naglik All Rights Reserved


#include "GAS/MMC_LightAttackDamage.h"
#include "GAS/ChrisAttributeSet.h"

UMMC_LightAttackDamage::UMMC_LightAttackDamage()
{
    // Same captures as MMC_BaseAttackDamage
    DamageCaptureDefinition.AttributeToCapture = UChrisAttributeSet::GetAttackDamageAttribute();
    DamageCaptureDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;

    ArmourCaptureDefinition.AttributeToCapture = UChrisAttributeSet::GetArmourAttribute();
    ArmourCaptureDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;

    // === NEW === Capture the light attack bonus from the attacker
    LightAttackBonusCaptureDefinition.AttributeToCapture = UChrisAttributeSet::GetLightAttackDamageBonusAttribute();
    LightAttackBonusCaptureDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;

    RelevantAttributesToCapture.Add(DamageCaptureDefinition);
    RelevantAttributesToCapture.Add(ArmourCaptureDefinition);
    RelevantAttributesToCapture.Add(LightAttackBonusCaptureDefinition);
}

float UMMC_LightAttackDamage::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    FAggregatorEvaluateParameters EvaluationParameters;
    EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

    float AttackDamage = 0.f;
    GetCapturedAttributeMagnitude(DamageCaptureDefinition, Spec, EvaluationParameters, AttackDamage);

    float Armour = 0.f;
    GetCapturedAttributeMagnitude(ArmourCaptureDefinition, Spec, EvaluationParameters, Armour);

    // === NEW === Read the light attack bonus (0.0 by default, 0.1 with Bladed Edge)
    float LightAttackBonus = 0.f;
    GetCapturedAttributeMagnitude(LightAttackBonusCaptureDefinition, Spec, EvaluationParameters, LightAttackBonus);

    // Base damage with armour reduction, then multiply by (1 + bonus)
    float Damage = AttackDamage * (1.f + LightAttackBonus) * (1.f - Armour / (Armour + 100.f));
    return -Damage;
}