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
	HoverGlow->SetVisibility(ESlateVisibility::Hidden);
}

void UTeamSelectionWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	HoverGlow->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UTeamSelectionWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	HoverGlow->SetVisibility(ESlateVisibility::Hidden);
}

void UTeamSelectionWidget::SelectButtonClicked()
{
	OnSlotClicked.Broadcast(SlotID);
}


