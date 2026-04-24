// Christopher Naglik All Rights Reserved


#include "GAS/MMC_LevelBased.h"
#include "MMC_LevelBased.h"
#include "GAS/CHeroAttributeSet.h"
#include "AbilitySystemComponent.h"

UMMC_LevelBased::UMMC_LevelBased()
{
	LevelCaptureDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	LevelCaptureDefinition.AttributeToCapture = UCHeroAttributeSet::GetLevelAttribute();

	RelevantAttributesToCapture.Add(LevelCaptureDefinition);
}

float UMMC_LevelBased::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	UAbilitySystemComponent* ASC = Spec.GetContext().GetInstigatorAbilitySystemComponent();
	if (!ASC)
	{
		return 0.0f;
	}

	float Level = 0;
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	GetCapturedAttributeMagnitude(LevelCaptureDefinition, Spec, EvaluationParameters, Level);

	bool bFound;
	float RateAttributeValue = ASC->GetGameplayAttributeValue(RateAttribute, bFound);
	if (!bFound)
		return 0.f;
	return (Level - 1) * RateAttributeValue;
}