// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ListView.h"
#include "GAS/ChrisGameplayAbilityTypes.h"
#include "AbilityListView.generated.h"

/**
 * 
 */
UCLASS()
class UAbilityListView : public UListView
{
	GENERATED_BODY()
	
public:
	void ConfigureAbilities(const TMap<EChrisAbilityInputID, TSubclassOf<class UGameplayAbility>>& Abilities);

	UPROPERTY(EditAnywhere, Category ="Data")
	UDataTable* AbilityDataTable;

	void AbilityGaugeGenerated(UUserWidget& Widget);

	const struct FAbilityWidgetData* FindWidgetDataForAbility(const TSubclassOf<UGameplayAbility>& AbilityClass) const;

};
