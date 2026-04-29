// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "StatsGauge.generated.h"

/**
 * 
 */
UCLASS()
class UStatsGauge : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	void BindToAttribute(UAbilitySystemComponent* ASC, const FGameplayAttribute& InAttribute);
	void SetValue(float NewValue);
	void AttributeChanged(const FOnAttributeChangeData& Data);

private:
	UPROPERTY(meta=(BindWidget))
	class UImage* Icon;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AttributeText;

	UPROPERTY(EditAnywhere, Category = "Attribute")
	FGameplayAttribute Attribute;

	UPROPERTY(EditAnywhere, Category = "Visual")
	UTexture2D* IconTexture;

	FNumberFormattingOptions NumberFormattingOptions;

	
};
