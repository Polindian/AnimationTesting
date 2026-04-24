// Christopher Naglik All Rights Reserved


#include "Widgets/StatsGauge.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "StatsGauge.h"

void UStatsGauge::NativePreConstruct()
{
	Super::NativePreConstruct();
	Icon->SetBrushFromTexture(IconTexture);
}

void UStatsGauge::NativeConstruct()
{
	Super::NativeConstruct();
	NumberFormattingOptions.MaximumFractionalDigits = 0;
	APawn* OwnerPlayerPawn = GetOwningPlayerPawn();
	if(!OwnerPlayerPawn)
	{
		return;
	}
	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerPlayerPawn);

	if (OwnerASC)
	{
		bool bFound;
		float AttributeValue = OwnerASC->GetGameplayAttributeValue(Attribute,bFound);
		SetValue(AttributeValue);

		OwnerASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &UStatsGauge::AttributeChanged);
	}

}

void UStatsGauge::SetValue(float NewValue)
{
	AttributeText->SetText(FText::AsNumber(NewValue, &NumberFormattingOptions));
}

void UStatsGauge::AttributeChanged(const FOnAttributeChangeData& Data)
{
	SetValue(Data.NewValue);
}
