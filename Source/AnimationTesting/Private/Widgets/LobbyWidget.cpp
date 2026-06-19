// Christopher Naglik All Rights Reserved


#include "Widgets/LobbyWidget.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Widgets/TeamSelectionWidget.h"
#include "Network/ChrisNetStatics.h"

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ClearAndPopulateTeamSelectionSlots();
}

void ULobbyWidget::ClearAndPopulateTeamSelectionSlots()
{
    RedTeamBox->ClearChildren();
    BlueTeamBox->ClearChildren();

    int32 PlayersPerTeam = UChrisNetStatics::GetPlayerCountPerTeam();

    for (int i = 0; i < PlayersPerTeam * 2; ++i)
    {
        UTeamSelectionWidget* NewSelectionSlot = CreateWidget<UTeamSelectionWidget>(this, TeamSelectionWidgetClass);
        if (NewSelectionSlot)
        {
            NewSelectionSlot->SetSlotID(i);

            // First half = Red team, second half = Blue team
            if (i < PlayersPerTeam)
            {
                RedTeamBox->AddChildToVerticalBox(NewSelectionSlot);
            }
            else
            {
                BlueTeamBox->AddChildToVerticalBox(NewSelectionSlot);
            }

            UVerticalBoxSlot* BoxSlot = Cast<UVerticalBoxSlot>(NewSelectionSlot->Slot);
            if (BoxSlot)
            {
                BoxSlot->SetPadding(FMargin(0.f, 15.f));
            }

            NewSelectionSlot->OnSlotClicked.AddUObject(this, &ULobbyWidget::SlotSelected);
            TeamSelectionSlots.Add(NewSelectionSlot);
        }
    }
}

void ULobbyWidget::SlotSelected(uint8 NewSlotId)
{
	UE_LOG(LogTemp, Log, TEXT("Attempted to switch to slot: %d"), NewSlotId);
}
