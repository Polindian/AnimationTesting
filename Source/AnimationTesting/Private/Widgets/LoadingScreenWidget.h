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

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Anim_ZoomIn;

	// Filled in the WBP's class defaults — one is picked at random per load
	UPROPERTY(EditDefaultsOnly, Category = "Hints", meta = (MultiLine = true))
	TArray<FText> Hints;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void HandleFadeOutFinished();

	UPROPERTY(Transient)
	class UAudioComponent* LoadingAudio = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	float AudioFadeInTime = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	float AudioFadeOutTime = 1.f;
};