// Christopher Naglik All Rights Reserved

#include "Widgets/SessionSearchBarWidget.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/Image.h"
#include "Audio/ChrisAudioSubsystem.h"
#include "Audio/ChrisGameplayTags.h"

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

void USessionSearchBarWidget::FocusBar(bool bPlaySound)
{
	if (!BarButton) { return; }

	bSuppressFocusSound = !bPlaySound;
	BarButton->SetFocus();
	bSuppressFocusSound = false;
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

	if (!InputText->HasKeyboardFocus())
	{
		FocusBar(true);
	}
}

void USessionSearchBarWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);

	if (bSuppressFocusSound)
	{
		bSuppressFocusSound = false;
	}
	else if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
	{
		Audio->Play2D(ChrisGameplayTags::Audio_UI_Navigate_Main);
	}

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
	// Only real typing reaches here — SetTextFromKeyboard broadcasts OnSearchTextChanged directly and deliberately skips this
	if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
	{
		Audio->Play2D(ChrisGameplayTags::Audio_UI_Navigate_Soft);
	}

	OnSearchTextChanged.Broadcast(Text);
}

void USessionSearchBarWidget::HandleTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		if (Text.ToString().Len() > MaxLength)
		{
			OnMaxLengthExceeded.Broadcast();
			return;   // rejection stays silent
		}

		if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
		{
			Audio->Play2D(ChrisGameplayTags::Audio_UI_Lobby_TeamSlot);
		}

		GetWorld()->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateWeakLambda(this, [this]() { FocusBar(); }));
	}
}