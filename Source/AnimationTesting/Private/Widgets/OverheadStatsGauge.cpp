// Christopher Naglik All Rights Reserved


#include "OverheadStatsGauge.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Widgets/ValueGauge.h"
#include "GAS/ChrisAttributeSet.h"


void UOverheadStatsGauge::ConfigureWithASC(UAbilitySystemComponent* AbilitySystemComponent)
{
	
	if (AbilitySystemComponent)
	{
		HealthBar->SetAndBoundGameplayAttribute(AbilitySystemComponent, UChrisAttributeSet::GetHealthAttribute(), UChrisAttributeSet::GetMaxHealthAttribute());
		ManaBar->SetAndBoundGameplayAttribute(AbilitySystemComponent, UChrisAttributeSet::GetManaAttribute(), UChrisAttributeSet::GetMaxManaAttribute());
	}
}
