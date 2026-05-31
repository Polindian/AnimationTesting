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
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "Visual")
    FName PercentMaterialParamName = "Percent";

    UPROPERTY(meta = (BindWidget))
    class UImage* LevelProgressImage;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* LevelText;

    // Smooth interpolation (same pattern as FlagCaptureGaugeWidget)
    float DisplayedPercent = 0.f;
    float TargetPercent = 0.f;

    UPROPERTY(EditDefaultsOnly, Category = "Visual")
    float InterpSpeed = 5.f;

    UPROPERTY()
    UMaterialInstanceDynamic* ProgressMaterial;

    FNumberFormattingOptions NumberFormattingOptions;

    const class UAbilitySystemComponent* OwnerASC;

    void UpdateGauge(const FOnAttributeChangeData& Data);
};
