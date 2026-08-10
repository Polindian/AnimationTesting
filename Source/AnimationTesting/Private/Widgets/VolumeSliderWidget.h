// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VolumeSliderWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnVolumeChanged, float);

UCLASS()
class ANIMATIONTESTING_API UVolumeSliderWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnVolumeChanged OnVolumeChanged;

	// Sets the slider and both visuals without broadcasting — for initial population
	void SetVolume(float NewVolume);

	float GetVolume() const;

	void FocusSlider();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class USlider> VolumeSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> FillBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> ValueText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Label;

	UPROPERTY(EditAnywhere, Category = "Volume Slider")
	FText LabelText;

	UPROPERTY(EditAnywhere, Category = "Volume Slider")
	FLinearColor ThumbNormalColor = FLinearColor(0.7f, 0.7f, 0.7f, 1.f);

	UPROPERTY(EditAnywhere, Category = "Volume Slider")
	FLinearColor ThumbFocusedColor = FLinearColor(1.f, 0.65f, 0.15f, 1.f);

	// How far one arrow press or stick nudge moves the value
	UPROPERTY(EditAnywhere, Category = "Volume Slider")
	float KeyStep = 0.05f;

private:
	UFUNCTION()
	void HandleSliderChanged(float NewValue);

	void RefreshVisuals(float Value);

	void NudgeVolume(float Delta);
};