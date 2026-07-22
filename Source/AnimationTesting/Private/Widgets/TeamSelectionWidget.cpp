// Christopher Naglik All Rights Reserved


#include "Widgets/TeamSelectionWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UTeamSelectionWidget::SetSlotID(uint8 NewSlotID)
{
	SlotID = NewSlotID;
}

void UTeamSelectionWidget::UpdateSlotInfo(const FString& PlayerNickname)
{
	InfoText->SetText(FText::FromString(PlayerNickname));
}

void UTeamSelectionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SelectButton->OnClicked.AddDynamic(this, &UTeamSelectionWidget::SelectButtonClicked);
	SelectButton->IsFocusable = true;

	HoverGlow->SetVisibility(ESlateVisibility::Hidden);
}

void UTeamSelectionWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	FocusSlot();
}

void UTeamSelectionWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
}

void UTeamSelectionWidget::SelectButtonClicked()
{
	OnSlotClicked.Broadcast(SlotID);
}

void UTeamSelectionWidget::SetReadyVisual(bool bReady)
{
	// Show green bar when ready, default bar when not
	GreenPlayerBar->SetVisibility(bReady ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	PlayerBar->SetVisibility(bReady ? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);
}

void UTeamSelectionWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);
	if (!HoverGlow) return;

	if (HoverGlow)
	{
		HoverGlow->SetVisibility(ESlateVisibility::Visible);
	}
}

void UTeamSelectionWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);
	if (!HoverGlow) return;

	if (HoverGlow)
	{
		HoverGlow->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UTeamSelectionWidget::FocusSlot()
{

	if (!SelectButton) return;

	if (SelectButton)
	{
		SelectButton->SetFocus();
	}
}

void UTeamSelectionWidget::SetDownNavigationTarget(UWidget* Target)
{
	if (SelectButton)
	{
		SelectButton->SetNavigationRuleExplicit(EUINavigation::Down, Target);
		SelectButton->BuildNavigation();
	}
}


