// Christopher Naglik All Rights Reserved

#include "Widgets/SessionEntryWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Audio/ChrisAudioSubsystem.h"
#include "Audio/ChrisGameplayTags.h"

void USessionEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SessionButton->OnClicked.AddDynamic(this, &USessionEntryWidget::SessionEntrySelected);

	// Navigation only considers focusable widgets
	SessionButton->IsFocusable = true;

	HoverGlow->SetVisibility(ESlateVisibility::Hidden);
}

void USessionEntryWidget::InitializeEntry(const FString& Name, const FString& SessionIdStr)
{
	SessionNameText->SetText(FText::FromString(Name));
	CachedSessionIdString = SessionIdStr;
}

void USessionEntryWidget::SessionEntrySelected()
{
	if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
	{
		Audio->Play2D(ChrisGameplayTags::Audio_UI_Lobby_TeamSlot);
	}

	OnSessionEntrySelected.Broadcast(CachedSessionIdString);
}

void USessionEntryWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	FocusEntry(true);
}

void USessionEntryWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);

	if (bSuppressFocusSound)
	{
		bSuppressFocusSound = false;
	}
	else if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
	{
		Audio->Play2D(ChrisGameplayTags::Audio_UI_Navigate_Soft);
	}

	if (HoverGlow)
	{
		HoverGlow->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void USessionEntryWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);
	if (HoverGlow)
	{
		HoverGlow->SetVisibility(ESlateVisibility::Hidden);
	}
}

void USessionEntryWidget::FocusEntry(bool bPlaySound)
{
	if (!SessionButton) { return; }

	bSuppressFocusSound = !bPlaySound;
	SessionButton->SetFocus();
	bSuppressFocusSound = false;
}

void USessionEntryWidget::SetSelectedVisual(bool bSelected)
{
	if (!SessionNameText) return;

	if (SessionNameText)
	{
		SessionNameText->SetColorAndOpacity(FSlateColor(bSelected ? SelectedTextColor : NormalTextColor));
	}
}