// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingScreenWidget.generated.h"

class UWidgetAnimation;

UCLASS()
class ANIMATIONTESTING_API ULoadingScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void ShowRandomHint();
	void DismissWithFade();

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HintText;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Anim_FadeOut;

	// Filled in the WBP's class defaults — one is picked at random per load
	UPROPERTY(EditDefaultsOnly, Category = "Hints", meta = (MultiLine = true))
	TArray<FText> Hints;

private:
	UFUNCTION()
	void HandleFadeOutFinished();
};