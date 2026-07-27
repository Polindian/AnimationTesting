// Christopher Naglik All Rights Reserved

#include "Widgets/SessionSearchBarWidget.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/Image.h"

void USessionSearchBarWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BarButton->IsFocusable = true;
	HighlightGlow->SetVisibility(ESlateVisibility::Hidden);

	InputText->OnTextChanged.AddDynamic(this, &USessionSearchBarWidget::HandleTextChanged);
	InputText->OnTextCommitted.AddDynamic(this, &USessionSearchBarWidget::HandleTextCommitted);
}

FText USessionSearchBarWidget::GetText() const
{
	return InputText->GetText();
}

void USessionSearchBarWidget::FocusBar()
{
	if (BarButton)
	{
		BarButton->SetFocus();
	}
}

void USessionSearchBarWidget::SetTextFromKeyboard(const FText& InText)
{
	InputText->SetText(InText);
	// SetText firing OnTextChanged is unreliable across engine versions — broadcast manually
	OnSearchTextChanged.Broadcast(InText);
	FocusBar();
}

UWidget* USessionSearchBarWidget::GetBarButton() const
{
	return BarButton;
}

void USessionSearchBarWidget::ClearText()
{
	InputText->SetText(FText::GetEmpty());
}

// Preview (tunneling) so we see Enter/A BEFORE the focused BarButton converts them
// into a button click — this is where keyboard and controller take different paths
FReply USessionSearchBarWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (BarButton && BarButton->HasKeyboardFocus())
	{
		const FKey Key = InKeyEvent.GetKey();

		// Keyboard: Enter on the highlighted bar -> start typing directly
		if (Key == EKeys::Enter)
		{
			InputText->SetKeyboardFocus();
			return FReply::Handled();
		}
		// Controller: A on the highlighted bar -> owner opens the on-screen keyboard.
		// This key only exists on gamepads, so no device detection is needed.
		if (Key == EKeys::Gamepad_FaceButton_Bottom)
		{
			OnVirtualKeyboardRequested.Broadcast();
			return FReply::Handled();
		}
	}
	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

void USessionSearchBarWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	// Hover highlights like every other control — but never steal focus mid-typing
	if (!InputText->HasKeyboardFocus())
	{
		FocusBar();
	}
}

void USessionSearchBarWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);
	if (HighlightGlow)
	{
		HighlightGlow->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void USessionSearchBarWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);
	if (HighlightGlow)
	{
		HighlightGlow->SetVisibility(ESlateVisibility::Hidden);
	}
}

void USessionSearchBarWidget::HandleTextChanged(const FText& Text)
{
	OnSearchTextChanged.Broadcast(Text);
}

void USessionSearchBarWidget::HandleTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		// Too long: don't refocus the bar — the owner opens the Continue dialog,
		// and its close handler brings focus back to the bar
		if (Text.ToString().Len() > MaxLength)
		{
			OnMaxLengthExceeded.Broadcast();
			return;
		}

		GetWorld()->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateWeakLambda(this, [this]() { FocusBar(); }));
	}
}