// Christopher Naglik All Rights Reserved

#include "Widgets/QualityOptionWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Audio/ChrisAudioSubsystem.h"
#include "Audio/ChrisGameplayTags.h"

void UQualityOptionWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetIsFocusable(true);
	OptionButton->IsFocusable = true;

	// Tinted here rather than in the asset, so the tick texture stays a plain
	// white checkmark and the colour matches the slider thumb
	if (TickIcon)
	{
		TickIcon->SetColorAndOpacity(TickColor);
		TickIcon->SetVisibility(ESlateVisibility::Hidden);
	}

	if (HoverGlow) { HoverGlow->SetVisibility(ESlateVisibility::Hidden); }
}

void UQualityOptionWidget::SetChosen(bool bChosen)
{
	if (TickIcon)
	{
		TickIcon->SetVisibility(bChosen ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}
}

void UQualityOptionWidget::FocusOption()
{
	if (OptionButton) { OptionButton->SetFocus(); }
}


// Mouse routes through focus so all three devices share one highlight path
void UQualityOptionWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	FocusOption();
}

void UQualityOptionWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);

	if (HoverGlow) { HoverGlow->SetVisibility(ESlateVisibility::HitTestInvisible); }

	if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
	{
		Audio->Play2D(ChrisGameplayTags::Audio_UI_Navigate_Soft);
	}
}

void UQualityOptionWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);

	if (HoverGlow) { HoverGlow->SetVisibility(ESlateVisibility::Hidden); }
}

FReply UQualityOptionWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (Key == EKeys::Enter || Key == EKeys::Gamepad_FaceButton_Bottom || Key == EKeys::Virtual_Accept)
	{
		HandleClicked();   
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UQualityOptionWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled();
}

FReply UQualityOptionWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}

	HandleClicked();
	return FReply::Handled();
}

void UQualityOptionWidget::HandleClicked()
{
	OnQualityOptionChosen.Broadcast(QualityLevel);
}