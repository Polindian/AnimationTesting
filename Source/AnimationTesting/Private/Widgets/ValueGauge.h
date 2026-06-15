// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "ValueGauge.generated.h"

UCLASS()
class UValueGauge : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void SetAndBoundGameplayAttribute(class UAbilitySystemComponent* AbilitySystemComponent, const FGameplayAttribute& Attribute, const FGameplayAttribute& MaxAttribute);
	void SetValue(float NewValue, float NewMaxValue);

	void SetBarColor(FLinearColor NewColor);
	void SetTextVisibility(bool bVisible);
	void SetDamageBarEnabled(bool bEnabled);

private:
	void ValueChanged(const FOnAttributeChangeData& ChangedData);
	void MaxValueChanged(const FOnAttributeChangeData& ChangedData);

	float CachedValue;
	float CachedMaxValue;

	// ---- Damage bar (yellow trail) ----
	bool bDamageBarEnabled = false;
	float DamageBarPercent = 1.f;
	float TargetPercent = 1.f;
	float DrainDelayRemaining = 0.f;

	UPROPERTY(EditAnywhere, Category = "Damage Bar")
	float DrainDelay = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Damage Bar")
	float DrainSpeed = 0.4f;

	// ---- Visuals ----
	UPROPERTY(EditAnywhere, Category = "Visual")
	FLinearColor BarColor;

	UPROPERTY(EditAnywhere, Category = "Visual")
	FSlateFontInfo ValueTextFont;

	UPROPERTY(EditAnywhere, Category = "Visual")
	bool bValueTextVisible = true;

	UPROPERTY(EditAnywhere, Category = "Visual")
	bool bProgressBarVisible = true;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget))
	class UProgressBar* ProgressBar;

	UPROPERTY(VisibleAnywhere, meta = (BindWidgetOptional))
	class UProgressBar* DamageBar;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget))
	class UTextBlock* ValueText;
};