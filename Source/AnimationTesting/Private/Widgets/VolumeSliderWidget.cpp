// Christopher Naglik All Rights Reserved


#include "VolumeSliderWidget.h"
#include "Components/Slider.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UVolumeSliderWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	VolumeSlider->OnValueChanged.AddDynamic(this, &UVolumeSliderWidget::HandleSliderChanged);
	VolumeSlider->SetSliderHandleColor(ThumbNormalColor);

	// Focus must land on the slider, not the wrapper, or arrow keys go nowhere
	SetIsFocusable(true);
	VolumeSlider->IsFocusable = false;

	if (Label) { Label->SetText(LabelText); }
}

float UVolumeSliderWidget::GetVolume() const
{
	return VolumeSlider ? VolumeSlider->GetValue() : 1.f;
}

void UVolumeSliderWidget::SetVolume(float NewVolume)
{
	// SetValue doesn't fire OnValueChanged, so this can't loop back into the handler
	VolumeSlider->SetValue(NewVolume);
	RefreshVisuals(NewVolume);
}

void UVolumeSliderWidget::FocusSlider()
{
	SetKeyboardFocus();
}

void UVolumeSliderWidget::HandleSliderChanged(float NewValue)
{
	RefreshVisuals(NewValue);
	OnVolumeChanged.Broadcast(NewValue);
}

void UVolumeSliderWidget::RefreshVisuals(float Value)
{
	if (FillBar) { FillBar->SetPercent(Value); }
	if (ValueText) { ValueText->SetText(FText::AsPercent(Value)); }
}

void UVolumeSliderWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);
	VolumeSlider->SetSliderHandleColor(ThumbFocusedColor);
}

void UVolumeSliderWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);
	VolumeSlider->SetSliderHandleColor(ThumbNormalColor);
}

void UVolumeSliderWidget::NudgeVolume(float Delta)
{
	const float NewValue = FMath::Clamp(GetVolume() + Delta, 0.f, 1.f);

	VolumeSlider->SetValue(NewValue);
	RefreshVisuals(NewValue);
	OnVolumeChanged.Broadcast(NewValue);   // SetValue is silent, so broadcast manually
}

FReply UVolumeSliderWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	// Handling all three devices here rather than relying on Slate's own key
	// handling: the gamepad stick and D-pad arrive as navigation and would
	// otherwise move focus off the slider instead of adjusting it
	if (Key == EKeys::Left || Key == EKeys::Gamepad_DPad_Left || Key == EKeys::Gamepad_LeftStick_Left)
	{
		NudgeVolume(-KeyStep);
		return FReply::Handled();
	}

	if (Key == EKeys::Right || Key == EKeys::Gamepad_DPad_Right || Key == EKeys::Gamepad_LeftStick_Right)
	{
		// Already at max — let the key through so navigation can move focus off
		// the slider, otherwise Right is swallowed forever and the quality
		// options are unreachable
		if (GetVolume() >= 1.f)
		{
			return FReply::Unhandled();
		}

		NudgeVolume(KeyStep);
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}