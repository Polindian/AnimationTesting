// Christopher Naglik All Rights Reserved


#include "Widgets/LobbyWidget.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/RetainerBox.h" 
#include "Components/TileView.h"
#include "Components/WidgetSwitcher.h"
#include "Character/PA_CharacterDefinition.h"
#include "Framework/ChrisGameState.h"
#include "Framework/CAssetManager.h"
#include "Widgets/TeamSelectionWidget.h"
#include "Widgets/CharacterEntryWidget.h"
#include "Network/ChrisNetStatics.h"
#include "Player/LobbyPlayerController.h"
#include "Player/ChrisPlayerState.h"


void ULobbyWidget::NativeConstruct()
{
    Super::NativeConstruct();
	ClearAndPopulateTeamSelectionSlots();

    // Cache our controller so we can call Server RPCs from the UI
	LobbyPlayerController = GetOwningPlayer<ALobbyPlayerController>();
    ConfigureGameState();

    ReadyUpButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnReadyUpClicked);
    ReadyUpButton->OnHovered.AddDynamic(this, &ULobbyWidget::OnReadyUpHovered);
    ReadyUpButton->OnUnhovered.AddDynamic(this, &ULobbyWidget::OnReadyUpUnhovered);

    if (LobbyPlayerController)
    {
        LobbyPlayerController->OnSwitchToHeroSelection.BindUObject(this, &ULobbyWidget::SwitchToHeroSelection);
    }

    UCAssetManager::Get().LoadCharacterDefinitions(FStreamableDelegate::CreateUObject(this, &ULobbyWidget::CharacterDefinitionsLoaded));
    if (CharacterSelectionTileView)
    {
        CharacterSelectionTileView->OnItemSelectionChanged().AddUObject(this, &ULobbyWidget::CharacterSelected);
    }
}

void ULobbyWidget::OnReadyUpHovered()
{
    if (ReadyUpRetainerDefault)
        ReadyUpRetainerDefault->SetEffectMaterial(HoverGradientMaterial);
}

void ULobbyWidget::OnReadyUpUnhovered()
{
    if (ReadyUpRetainerDefault)
        ReadyUpRetainerDefault->SetEffectMaterial(nullptr);
}

void ULobbyWidget::OnReadyUpClicked()
{
    SetReadyState(!bIsReady);
}

void ULobbyWidget::SetReadyState(bool bReady)
{
    bIsReady = bReady;
    ReadyUpText->SetText(FText::FromString(bReady ? TEXT("UNREADY") : TEXT("READY UP")));

    // Tell the server so all clients see the green bar
    if (LobbyPlayerController)
    {
        LobbyPlayerController->Server_RequestReadyStateChange(bReady);
    }
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

            // Subscribe to click events so we know when this slot is selected
            NewSelectionSlot->OnSlotClicked.AddUObject(this, &ULobbyWidget::SlotSelected);
            TeamSelectionSlots.Add(NewSelectionSlot);
        }
    }
}

// Called when any slot widget is clicked — sends the request to the server via RPC
void ULobbyWidget::SlotSelected(uint8 NewSlotId)
{
    // Can't switch slots while readied up
    if (bIsReady) return;
    
    UE_LOG(LogTemp, Log, TEXT("Attempted to switch to slot: %d"), NewSlotId);
    if(LobbyPlayerController)
    {
        LobbyPlayerController->Server_RequestSlotSelectionChange(NewSlotId);
	}
}

// Attempts to find and subscribe to the GameState. Retries on a timer if not yet available (network delay).
void ULobbyWidget::ConfigureGameState()
{
    UWorld* World = GetWorld();
    if (!World) return;

    ChrisGameState = World->GetGameState<AChrisGameState>();
    if (!ChrisGameState)
    {
        World->GetTimerManager().SetTimer(ConfigureGameStateTimerHandle, this, &ULobbyWidget::ConfigureGameState, 1.f);
    }
    else
    {
        // Subscribe to future updates, then immediately display current state
        ChrisGameState->OnPlayerSelectionUpdated.AddUObject(this, &ULobbyWidget::UpdatePlayerSelectionDisplay);
        UpdatePlayerSelectionDisplay(ChrisGameState->GetPlayerSelection());
    }
}

// Refreshes all slot widgets: resets to "Unoccupied", then fills in occupied slots with player names
void ULobbyWidget::UpdatePlayerSelectionDisplay(const TArray<FPlayerSelection>& PlayerSelections)
{
    // Pass 1: reset all slots
    for (UTeamSelectionWidget* SelectionSlot : TeamSelectionSlots)
    {
        SelectionSlot->UpdateSlotInfo("Unoccupied");
        SelectionSlot->SetReadyVisual(false);
    }

    // Reset all character entry icons to deselected (desaturated)
    for (UUserWidget* CharacterEntryAsWidget : CharacterSelectionTileView->GetDisplayedEntryWidgets())
    {
        if (UCharacterEntryWidget* CharacterEntryWidget = Cast<UCharacterEntryWidget>(CharacterEntryAsWidget))
        {
            CharacterEntryWidget->SetSelected(false);
        }
    }

    // Pass 2: fill occupied slots with name and ready state
    for (const FPlayerSelection& PlayerSelection : PlayerSelections)
    {
        if (!PlayerSelection.IsValid()) continue;

        uint8 SlotIndex = PlayerSelection.GetPlayerSlot();
        TeamSelectionSlots[SlotIndex]->UpdateSlotInfo(PlayerSelection.GetPlayerNickname());
        TeamSelectionSlots[SlotIndex]->SetReadyVisual(PlayerSelection.GetIsReady());

        // Only highlight the character icon for the LOCAL player's pick
        if (PlayerSelection.IsForPlayer(GetOwningPlayerState()))
        {
            UCharacterEntryWidget* SelectedEntry = CharacterSelectionTileView->GetEntryWidgetFromItem<UCharacterEntryWidget>(PlayerSelection.GetCharacterDefinition());
            if (SelectedEntry)
            {
                SelectedEntry->SetSelected(true);
            }
        }
    }
}

// Server told us to switch — change the widget switcher to hero selection page
void ULobbyWidget::SwitchToHeroSelection()
{
    MainSwitcher->SetActiveWidget(HeroSelectionRoot);
}

void ULobbyWidget::CharacterDefinitionsLoaded()
{
    TArray<UPA_CharacterDefinition*> LoadedCharacterDefinitions;
    if (UCAssetManager::Get().GetLoadedCharacterDefinitions(LoadedCharacterDefinitions))
    {
        for (UPA_CharacterDefinition* LoadedCharacterDefinition : LoadedCharacterDefinitions)
        {
            UE_LOG(LogTemp, Warning, TEXT("Loaded Character: %s"), *(LoadedCharacterDefinition->GetCharacterDisplayName()));
            CharacterSelectionTileView->SetListItems(LoadedCharacterDefinitions);
        }
    }
}

void ULobbyWidget::CharacterSelected(UObject* SelectedUObject)
{
    if (!ChrisPlayerState)
    {
        ChrisPlayerState = GetOwningPlayerState<AChrisPlayerState>();
    }

    if (!ChrisPlayerState)
        return;

    if (const UPA_CharacterDefinition* CharacterDefinition = Cast<UPA_CharacterDefinition>(SelectedUObject))
    {
        ChrisPlayerState->Server_SetSelectedCharacterDefinition(CharacterDefinition);
    }
}
