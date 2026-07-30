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

	DoubleDownChanceCaptureDefinition.AttributeToCapture = UChrisAttributeSet::GetDoubleDownChanceAttribute();
	DoubleDownChanceCaptureDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;
	DoubleDownChanceCaptureDefinition.bSnapshot = false;
	RelevantAttributesToCapture.Add(DoubleDownChanceCaptureDefinition);
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

	// Read crit chance (0.0 if Double Down not purchased, 0.2 if purchased)
	float CritChance = 0.f;
	GetCapturedAttributeMagnitude(DoubleDownChanceCaptureDefinition, Spec, EvaluationParameters, CritChance);

	// Roll for crit — 20% chance to deal 30% more damage on combo hits
	// Safe because combo damage is only applied server-side (K2_HasAuthority())
	float CritMultiplier = 1.f;
	if (CritChance > 0.f && FMath::FRand() < CritChance)
	{
		CritMultiplier = 1.3f;
		UE_LOG(LogTemp, Warning, TEXT("[DoubleDown] CRIT! Combo damage multiplied by 1.3x"));
	}

	float Damage = AttackDamage * CritMultiplier * (1.f - Armour / (Armour + 100.f));
	return -Damage;
}
