// Christopher Naglik All Rights Reserved

#include "Widgets/SessionEntryWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

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
	OnSessionEntrySelected.Broadcast(CachedSessionIdString);
}

void USessionEntryWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	FocusEntry();
}

void USessionEntryWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);
	if (HoverGlow)
	{
		// HitTestInvisible, never Visible — the glow must not steal clicks from the button
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

void USessionEntryWidget::FocusEntry()
{
	if (SessionButton)
	{
		SessionButton->SetFocus();
	}
}

void USessionEntryWidget::SetSelectedVisual(bool bSelected)
{
	if (!SessionNameText) return;

	if (SessionNameText)
	{
		SessionNameText->SetColorAndOpacity(FSlateColor(bSelected ? SelectedTextColor : NormalTextColor));
	}
}