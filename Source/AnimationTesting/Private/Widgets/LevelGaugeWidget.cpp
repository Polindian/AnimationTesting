// Christopher Naglik All Rights Reserved


#include "Widgets/LevelGaugeWidget.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/CHeroAttributeSet.h"


void ULevelGaugeWidget::NativeConstruct()
{
	Super::NativeConstruct();
	NumberFormattingOptions.SetMaximumFractionalDigits(0);

	APawn* OwnerPawn = GetOwningPlayerPawn();
	if (!OwnerPawn)
	{
		return;
	}

	UAbilitySystemComponent* OwnerAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerPawn);
	if(!OwnerAbilitySystemComponent)
	{
		return;
	}
	OwnerASC = OwnerAbilitySystemComponent;

	UpdateGauge(FOnAttributeChangeData());
	OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetExperienceAttribute()).AddUObject(this, &ULevelGaugeWidget::UpdateGauge);
	OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetNextLevelExperienceAttribute()).AddUObject(this, &ULevelGaugeWidget::UpdateGauge);
	OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetPrevLevelExperienceAttribute()).AddUObject(this, &ULevelGaugeWidget::UpdateGauge);
	OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetLevelAttribute()).AddUObject(this, &ULevelGaugeWidget::UpdateGauge);

}

void ULevelGaugeWidget::UpdateGauge(const FOnAttributeChangeData& Data)
{
	bool bFound;
	float CurrentExperience = OwnerASC->GetGameplayAttributeValue(UCHeroAttributeSet::GetExperienceAttribute(), bFound);
	if (!bFound)
		return;
	float NextLevelExperience = OwnerASC->GetGameplayAttributeValue(UCHeroAttributeSet::GetNextLevelExperienceAttribute(), bFound);
	if (!bFound)
		return;
	float PrevLevelExperience = OwnerASC->GetGameplayAttributeValue(UCHeroAttributeSet::GetPrevLevelExperienceAttribute(), bFound);
	if (!bFound)
		return;
	float CurrentLevel = OwnerASC->GetGameplayAttributeValue(UCHeroAttributeSet::GetLevelAttribute(), bFound);
	if (!bFound)
		return;

	LevelText->SetText(FText::AsNumber(CurrentLevel, &NumberFormattingOptions));
	float Progress = CurrentExperience - PrevLevelExperience;
	float LevelExperienceAmountLeft = NextLevelExperience - PrevLevelExperience;

	float Percent = Progress / LevelExperienceAmountLeft;

	if (NextLevelExperience == 0)
	{
		Percent = 1;
	}

	if (LevelProgressImage)
	{
		LevelProgressImage->GetDynamicMaterial()->SetScalarParameterValue(PercentMaterialParamName, Percent);
	}
}

