// Christopher Naglik All Rights Reserved


#include "Widgets/AbilityListView.h"
#include "Abilities/GameplayAbility.h"
#include "Widgets/AbilityGauge.h"


void UAbilityListView::ConfigureAbilities(const TMap<EChrisAbilityInputID, TSubclassOf<UGameplayAbility>>& Abilities)
{
	OnEntryWidgetGenerated().AddUObject(this, &UAbilityListView::AbilityGaugeGenerated);
	
	for (const TPair<EChrisAbilityInputID, TSubclassOf<UGameplayAbility>>& AbilityPair : Abilities)
	{
		AddItem(AbilityPair.Value.GetDefaultObject());
	}
}

void UAbilityListView::AbilityGaugeGenerated(UUserWidget& Widget)
{
	UAbilityGauge* AbilityGauge = Cast<UAbilityGauge>(&Widget);

	if (AbilityGauge)
	{
		AbilityGauge->ConfigureWithWidgetData(FindWidgetDataForAbility(AbilityGauge->GetListItem<UGameplayAbility>()->GetClass()));
	}
}

const FAbilityWidgetData* UAbilityListView::FindWidgetDataForAbility(const TSubclassOf<UGameplayAbility>& AbilityClass) const
{
	if (!AbilityDataTable) return nullptr;

	for (auto& AbilityWidgetDataPair : AbilityDataTable->GetRowMap())
	{
		const FAbilityWidgetData* WidgetData = AbilityDataTable->FindRow<FAbilityWidgetData>(AbilityWidgetDataPair.Key, "");
		if (WidgetData->AbilityClass == AbilityClass)
		{
			return WidgetData;
		}
	}

	return nullptr;
}

void UAbilityListView::ResetAllCooldownVisuals()
{
	for (int32 i = 0; i < GetNumItems(); i++)
	{
		UUserWidget* EntryWidget = GetEntryWidgetFromItem(GetItemAt(i));
		if (UAbilityGauge* Gauge = Cast<UAbilityGauge>(EntryWidget))
		{
			Gauge->StartRoundCooldown(); 
		}
	}
}