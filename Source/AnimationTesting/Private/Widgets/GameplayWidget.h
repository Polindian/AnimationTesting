// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GAS/ChrisGameplayAbilityTypes.h"
#include "Abilities/GameplayAbility.h" 
#include "GameplayWidget.generated.h"

/**
 * 
 */
UCLASS()
class UGameplayWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	void ConfigureAbilities(const TMap<EChrisAbilityInputID, TSubclassOf<class UGameplayAbility>>& Abilities);

private:
	UPROPERTY(meta = (BindWidget))
	class UValueGauge* HealthBar;

	UPROPERTY(meta = (BindWidget))
	class UValueGauge* ManaBar;

	UPROPERTY(meta = (BindWidget))
	class UStatsGauge* AttackDamageGauge;

	UPROPERTY(meta = (BindWidget))
	class UStatsGauge* ArmorGauge;

	UPROPERTY(meta = (BindWidget))
	class UStatsGauge* MoveSpeedGauge;

	UPROPERTY(meta = (BindWidget))
	class UStatsGauge* IntelligenceGauge;

	UPROPERTY(meta = (BindWidget))
	class UStatsGauge* StrengthGauge;

	UPROPERTY()
	class UAbilitySystemComponent* OwnerAbilitySystemComponent;

	UPROPERTY(meta = (BindWidget))
	class UAbilityListView* AbilityListView;

	UPROPERTY(meta=(BindWidget))
	class UCrosshairWidget* CrosshairWidget;
};
