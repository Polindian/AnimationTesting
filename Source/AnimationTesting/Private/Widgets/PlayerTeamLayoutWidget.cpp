// Christopher Naglik All Rights Reserved


#include "Widgets/PlayerTeamLayoutWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Widgets/PlayerTeamSlotWidget.h"
#include "Network/ChrisNetStatics.h"

void UPlayerTeamLayoutWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RedTeamLayoutBox->ClearChildren();
	BlueTeamLayoutBox->ClearChildren();

	if (!PlayerTeamSlotWidgetClass)
		return;

	for (int i = 0; i < UChrisNetStatics::GetPlayerCountPerTeam() * 2; ++i)
	{
		UPlayerTeamSlotWidget* NewSlotWidget = CreateWidget<UPlayerTeamSlotWidget>(GetOwningPlayer(), PlayerTeamSlotWidgetClass);
		TeamSlotWidgets.Add(NewSlotWidget);

		UHorizontalBoxSlot* NewSlot;
		if (i < UChrisNetStatics::GetPlayerCountPerTeam())
		{
			NewSlot = RedTeamLayoutBox->AddChildToHorizontalBox(NewSlotWidget);
		}
		else
		{
			NewSlot = BlueTeamLayoutBox->AddChildToHorizontalBox(NewSlotWidget);
		}

		NewSlot->SetPadding(FMargin{ PlayerTeamWidgetSlotMargin });
	}

}

void UPlayerTeamLayoutWidget::UpdatePlayerSelection(const TArray<FPlayerSelection>& PlayerSelections)
{
	// Pass 1: reset all slots to empty (no name, no icon)
	for (UPlayerTeamSlotWidget* SlotWidget : TeamSlotWidgets)
	{
		SlotWidget->UpdateSlot("", nullptr);
	}

	// Pass 2: fill in each occupied slot with the player's name and character pick
	for (const FPlayerSelection& PlayerSelection : PlayerSelections)
	{
		if (!PlayerSelection.IsValid())
			continue;

		uint8 SlotIndex = PlayerSelection.GetPlayerSlot();
		TeamSlotWidgets[PlayerSelection.GetPlayerSlot()]->UpdateSlot(PlayerSelection.GetPlayerNickname(), PlayerSelection.GetCharacterDefinition());

		// Drive the green/white name color based on whether this player has locked in
		TeamSlotWidgets[SlotIndex]->SetLockedInVisual(PlayerSelection.GetIsLockedIn());
	}
}
