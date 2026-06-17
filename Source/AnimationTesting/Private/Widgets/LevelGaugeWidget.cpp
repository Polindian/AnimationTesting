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

	// Cache the dynamic material instance
	if (LevelProgressImage)
	{
		ProgressMaterial = LevelProgressImage->GetDynamicMaterial();
	}

	UpdateGauge(FOnAttributeChangeData());
	OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetExperienceAttribute()).AddUObject(this, &ULevelGaugeWidget::UpdateGauge);
	OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetNextLevelExperienceAttribute()).AddUObject(this, &ULevelGaugeWidget::UpdateGauge);
	OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetPrevLevelExperienceAttribute()).AddUObject(this, &ULevelGaugeWidget::UpdateGauge);
	OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetLevelAttribute()).AddUObject(this, &ULevelGaugeWidget::UpdateGauge);

}

void ULevelGaugeWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Smoothly interpolate the displayed percent toward the target
	DisplayedPercent = FMath::FInterpTo(DisplayedPercent, TargetPercent, InDeltaTime, InterpSpeed);

	if (ProgressMaterial)
	{
		ProgressMaterial->SetScalarParameterValue(PercentMaterialParamName, DisplayedPercent);
	}
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

	// Update level number text
	LevelText->SetText(FText::AsNumber(CurrentLevel, &NumberFormattingOptions));

	// Calculate target percent — the tick will smoothly interpolate toward this
	float Progress = CurrentExperience - PrevLevelExperience;
	float LevelRange = NextLevelExperience - PrevLevelExperience;

	TargetPercent = (LevelRange > 0.f) ? (Progress / LevelRange) : 1.f;
}

