// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadStatsGauge.generated.h"

/**
 * 
 */
UCLASS()
class UOverheadStatsGauge : public UUserWidget
{
	GENERATED_BODY()
public:
	void ConfigureWithASC(class UAbilitySystemComponent* AbilitySystemComponent);

	void ConfigureTeamColor(AActor* OwnerActor);

	void SetTextVisibility(bool bVisible);
	

private:
	UPROPERTY(meta = (BindWidget))
	class UValueGauge* HealthBar;
	UPROPERTY(meta = (BindWidget))
	class UValueGauge* ManaBar;
};
