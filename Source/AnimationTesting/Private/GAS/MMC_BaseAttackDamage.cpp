// Christopher Naglik All Rights Reserved


#include "GAS/MMC_BaseAttackDamage.h"
#include "GAS/ChrisAttributeSet.h"

UMMC_BaseAttackDamage::UMMC_BaseAttackDamage()
{
	DamageCaptureDefinition.AttributeToCapture = UChrisAttributeSet::GetAttackDamageAttribute();
	DamageCaptureDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;

	ArmourCaptureDefinition.AttributeToCapture = UChrisAttributeSet::GetArmourAttribute();
	ArmourCaptureDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;

	RelevantAttributesToCapture.Add(DamageCaptureDefinition);
	RelevantAttributesToCapture.Add(ArmourCaptureDefinition);
}

float UMMC_BaseAttackDamage::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	float AttackDamage = 0.f;
	GetCapturedAttributeMagnitude(DamageCaptureDefinition, Spec, EvaluationParameters, AttackDamage);

	float Armour = 0.f;
	GetCapturedAttributeMagnitude(ArmourCaptureDefinition, Spec, EvaluationParameters, Armour);

	float Damage = AttackDamage * (1 - Armour / (Armour + 100));
	return -Damage;
}
