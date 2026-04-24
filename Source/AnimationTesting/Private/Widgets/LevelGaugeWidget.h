// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "LevelGaugeWidget.generated.h"

/**
 * 
 */
UCLASS()
class ULevelGaugeWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FName PercentMaterialParamName = "Percent";

	UPROPERTY(meta=(BindWidget))
	class UImage* LevelProgressImage;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* LevelText;

	FNumberFormattingOptions NumberFormattingOptions;

	const class UAbilitySystemComponent* OwnerASC;

	void UpdateGauge(const FOnAttributeChangeData& Data);
};
