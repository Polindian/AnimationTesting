// Christopher Naglik All Rights Reserved


#include "Widgets/LobbyWidget.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/RetainerBox.h" 
#include "Components/TileView.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Character/PA_CharacterDefinition.h"
#include "GameFramework/PlayerStart.h"
#include "Framework/ChrisGameState.h"
#include "Framework/CAssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/TeamSelectionWidget.h"
#include "Widgets/CharacterEntryWidget.h"
#include "Widgets/CharacterDisplay.h"
#include "Widgets/PlayerTeamLayoutWidget.h"
#include "Widgets/MenuButtonWidget.h"
#include "Network/ChrisNetStatics.h"
#include "Player/LobbyPlayerController.h"
#include "Player/ChrisPlayerState.h"
#include "Audio/ChrisAudioSubsystem.h"
#include "Audio/ChrisGameplayTags.h"
#include "Widgets/GeneralMenuWidget.h"
#include "Framework/ChrisGameInstance.h"
#include "Camera/PlayerCameraManager.h"
#include "Misc/PackageName.h"


void ULobbyWidget::NativeConstruct()
{
    Super::NativeConstruct();
	ClearAndPopulateTeamSelectionSlots();

    GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
            {
                if (TeamSelectionSlots.Num() > 0 && TeamSelectionSlots[0])
                {
                    TeamSelectionSlots[0]->FocusSlot();
                }
            }));

    // Cache our controller so we can call Server RPCs from the UI
	LobbyPlayerController = GetOwningPlayer<ALobbyPlayerController>();
    ConfigureGameState();

    ReadyUpButton->OnMenuButtonClicked.AddDynamic(this, &ULobbyWidget::OnReadyUpClicked);

    if (LobbyPlayerController)
    {
        LobbyPlayerController->OnSwitchToHeroSelection.BindUObject(this, &ULobbyWidget::SwitchToHeroSelection);
    }

    UCAssetManager::Get().LoadCharacterDefinitions(FStreamableDelegate::CreateUObject(this, &ULobbyWidget::CharacterDefinitionsLoaded));
    if (CharacterSelectionTileView)
    {
        CharacterSelectionTileView->OnEntryWidgetGenerated().AddUObject(this, &ULobbyWidget::HandleHeroEntryGenerated);
    }

    SpawnCharacterDisplay();

    if (StartMatchButton)
    {
        StartMatchButton->OnMenuButtonClicked.AddDynamic(this, &ULobbyWidget::OnStartMatchButtonClicked);
    }
}

void ULobbyWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Update the hero selection countdown display from the server's replicated timer
    if (ChrisGameState && HeroSelectionTimerText)
    {
        int32 TimeLeft = FMath::CeilToInt(ChrisGameState->GetHeroSelectionTimeRemaining());
        HeroSelectionTimerText->SetText(FText::FromString(FString::Printf(TEXT("%d"), FMath::Max(0, TimeLeft))));
    }
}

void ULobbyWidget::OnReadyUpClicked()
{
    // bIsReady is still the pre-click state here
    const bool bWillBeReady = !bIsReady;

    if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
    {
        Audio->Play2D(bWillBeReady
            ? ChrisGameplayTags::Audio_UI_Lobby_Continue
            : ChrisGameplayTags::Audio_UI_Leaderboard_Close);
    }

    SetReadyState(bWillBeReady);
}

void ULobbyWidget::SetReadyState(bool bReady)
{
    bIsReady = bReady;
    ReadyUpButton->SetButtonText(FText::FromString(bReady ? TEXT("UNREADY") : TEXT("READY UP")));

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

    // Spatial nav can't find ReadyUp from the left column — wire both bottom slots to it explicitly
    UWidget* ReadyTarget = ReadyUpButton->GetMainButton();
    TeamSelectionSlots[PlayersPerTeam - 1]->SetDownNavigationTarget(ReadyTarget);          // bottom of Red
    TeamSelectionSlots[TeamSelectionSlots.Num() - 1]->SetDownNavigationTarget(ReadyTarget); // bottom of Blue
}

// Called when any slot widget is clicked — sends the request to the server via RPC
void ULobbyWidget::SlotSelected(uint8 NewSlotId)
{
    // Can't switch slots while readied up
    UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this);

    // Can't switch slots while readied up
    if (bIsReady)
    {
        if (Audio) { Audio->Play2D(ChrisGameplayTags::Audio_UI_Reject); }
        return;
    }

    if (Audio) { Audio->Play2D(ChrisGameplayTags::Audio_UI_Lobby_TeamSlot); }

    UE_LOG(LogTemp, Log, TEXT("Attempted to switch to slot: %d"), NewSlotId);
    if (LobbyPlayerController)
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
    if (!CharacterSelectionTileView || !IsValid(CharacterSelectionTileView))
        return;

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

            UpdateCharacterDisplay(PlayerSelection);

            // Sync local lock-in state with what the server says (handles force-lock from timer expiry)
            bIsLockedIn = PlayerSelection.GetIsLockedIn();

            // When locked in, disable the character selection so they can't change picks
            if (CharacterSelectionTileView)
            {
                CharacterSelectionTileView->SetRenderOpacity(bIsLockedIn ? 0.4f : 1.f);
            }

            if (StartMatchButton)
            {
                StartMatchButton->SetButtonText(FText::FromString(bIsLockedIn ? TEXT("RETURN") : TEXT("START MATCH")));
            }
        }
    }

    if (PlayerTeamLayoutWidget)
    {
        PlayerTeamLayoutWidget->UpdatePlayerSelection(PlayerSelections);
    }
}

// Server told us to switch — change the widget switcher to hero selection page
void ULobbyWidget::SwitchToHeroSelection()
{
    MainSwitcher->SetActiveWidget(HeroSelectionRoot);
    HeroTooltipImage->SetVisibility(ESlateVisibility::Hidden);

    // Default focus: the character grid, so controller users can immediately navigate tiles with the stick/D-pad and pick with A/Enter
    if (CharacterSelectionTileView)
    {
        TryInitHeroSelectionFocus();
    }

    // Turn on the character spotlight — starts with Visible=false in the level
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("HeroSelectionSpotLight"), FoundActors);
    for (AActor* Actor : FoundActors)
    {
        Actor->GetRootComponent()->SetVisibility(true, true);
    }
}

void ULobbyWidget::CharacterDefinitionsLoaded()
{
    TArray<UPA_CharacterDefinition*> LoadedCharacterDefinitions;
    if (UCAssetManager::Get().GetLoadedCharacterDefinitions(LoadedCharacterDefinitions))
    {
        CharacterSelectionTileView->SetListItems(LoadedCharacterDefinitions);   // moved out of the loop

        // Definitions are loaded but their meshes/grooms/textures are still soft references. Warm them now, well before anyone reaches hero selection.
        PreloadHeroAssets(LoadedCharacterDefinitions);
    }
}




void ULobbyWidget::WireHeroSelectionNavigation()
{
    UWidget* StartBtn = StartMatchButton->GetMainButton();

    UCharacterEntryWidget* FirstEntry = nullptr;
    for (UUserWidget* W : CharacterSelectionTileView->GetDisplayedEntryWidgets())
    {
        UCharacterEntryWidget* Entry = Cast<UCharacterEntryWidget>(W);
        if (!Entry) continue;
        if (!FirstEntry) { FirstEntry = Entry; }

        Entry->GetSelectButton()->SetNavigationRuleExplicit(EUINavigation::Right, StartBtn);
        Entry->GetSelectButton()->BuildNavigation();
    }

    // Left from StartMatch back into the grid — but never at disabled tiles (locked in)
    if (FirstEntry && !bIsLockedIn)
    {
        StartBtn->SetNavigationRuleExplicit(EUINavigation::Left, FirstEntry->GetSelectButton());
    }
    else
    {
        StartBtn->SetNavigationRuleBase(EUINavigation::Left, EUINavigationRule::Stop);
    }
    StartBtn->BuildNavigation();
}

void ULobbyWidget::TryInitHeroSelectionFocus()
{
    // Tiles only generate once the page is active and laid out — retry until they exist
    if (!CharacterSelectionTileView || CharacterSelectionTileView->GetDisplayedEntryWidgets().Num() == 0)
    {
        GetWorld()->GetTimerManager().SetTimerForNextTick(
            FTimerDelegate::CreateWeakLambda(this, [this]() { TryInitHeroSelectionFocus(); }));
        return;
    }

    WireHeroSelectionNavigation();

    for (UUserWidget* W : CharacterSelectionTileView->GetDisplayedEntryWidgets())
    {
        if (UCharacterEntryWidget* First = Cast<UCharacterEntryWidget>(W))
        {
            First->FocusEntry();     // already silent by default
            HeroEntryClicked(First->GetCharacterDefinition(), false);
            break;
        }
    }
}



void ULobbyWidget::SpawnCharacterDisplay()
{
    if (CharacterDisplay)
        return;

    if (!CharacterDisplayClass)
        return;

    FTransform CharacterDisplayTransform = FTransform::Identity;
    AActor* PlayerStart = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass());
    if (PlayerStart)
    {
        CharacterDisplayTransform = PlayerStart->GetActorTransform();
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    CharacterDisplay = GetWorld()->SpawnActor<ACharacterDisplay>(CharacterDisplayClass, SpawnParams);
    GetOwningPlayer()->SetViewTarget(CharacterDisplay);
}

void ULobbyWidget::UpdateCharacterDisplay(const FPlayerSelection& PlayerSelection)
{
    if (!CharacterDisplay || !IsValid(CharacterDisplay))
        return;

    if (!PlayerSelection.GetCharacterDefinition())
        return;

    if (PlayerSelection.GetCharacterDefinition() == CurrentDisplayedDefinition)
        return;

    CurrentDisplayedDefinition = PlayerSelection.GetCharacterDefinition();
    CharacterDisplay->ConfigureWithCharacterDefinition(CurrentDisplayedDefinition);
}

// Toggles lock-in state: START MATCH → locks in, RETURN → unlocks
void ULobbyWidget::OnStartMatchButtonClicked()
{
    if (!LobbyPlayerController) return;

    // bIsLockedIn is still the pre-click state here
    const bool bWillBeLockedIn = !bIsLockedIn;

    if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
    {
        Audio->Play2D(bWillBeLockedIn
            ? ChrisGameplayTags::Audio_UI_Lobby_Continue
            : ChrisGameplayTags::Audio_UI_Leaderboard_Close);
    }

    bIsLockedIn = bWillBeLockedIn;
    LobbyPlayerController->Server_RequestLockIn(bIsLockedIn);
}

void ULobbyWidget::HandleHeroEntryGenerated(UUserWidget& EntryWidget)
{
    if (UCharacterEntryWidget* Entry = Cast<UCharacterEntryWidget>(&EntryWidget))
    {
        Entry->OnEntryHovered.RemoveAll(this);
        Entry->OnEntryClicked.RemoveAll(this);
        Entry->OnEntryHovered.AddUObject(this, &ULobbyWidget::HeroEntryHovered);
        Entry->OnEntryClicked.AddUObject(this, &ULobbyWidget::HeroEntryClicked, true);
    }
}

void ULobbyWidget::HeroEntryHovered(const UPA_CharacterDefinition* Definition)
{
    ShowHeroTooltip(Definition);
}

void ULobbyWidget::ShowHeroTooltip(const UPA_CharacterDefinition* Definition)
{
    if (!HeroTooltipImage || !Definition) return;

    if (UTexture2D* TooltipTexture = Definition->LoadToolTip())
    {
        HeroTooltipImage->SetBrushFromTexture(TooltipTexture);
        HeroTooltipImage->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
}

void ULobbyWidget::PreloadHeroAssets(const TArray<UPA_CharacterDefinition*>& Definitions)
{
    TArray<FSoftObjectPath> PathsToLoad;
    for (const UPA_CharacterDefinition* Def : Definitions)
    {
        if (Def) { Def->GetPreloadAssetPaths(PathsToLoad); }
    }

    if (PathsToLoad.Num() == 0) return;

    // Handle is stored so the assets stay loaded — if it goes out of scope they can be garbage collected
    HeroAssetPreloadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
        PathsToLoad,
        FStreamableDelegate::CreateWeakLambda(this, [this]()
            {
                UE_LOG(LogTemp, Warning, TEXT("Hero display assets preloaded"));
            }));
}

void ULobbyWidget::HeroEntryClicked(const UPA_CharacterDefinition* Definition, bool bPlaySound)
{
    UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this);

    // Locked in: the tiles are dimmed but still clickable, so tell the player no
    if (bIsLockedIn)
    {
        if (bPlaySound && Audio) { Audio->Play2D(ChrisGameplayTags::Audio_UI_Reject); }
        return;
    }

    if (!Definition) return;

    // Already picked this one — reject rather than re-confirming
    if (Definition == CurrentDisplayedDefinition)
    {
        if (bPlaySound && Audio) { Audio->Play2D(ChrisGameplayTags::Audio_UI_Reject); }
        return;
    }

    if (!ChrisPlayerState)
    {
        ChrisPlayerState = GetOwningPlayerState<AChrisPlayerState>();
    }
    if (!ChrisPlayerState) return;

    if (bPlaySound && Audio) { Audio->Play2D(ChrisGameplayTags::Audio_UI_Lobby_TeamSlot); }

    ChrisPlayerState->Server_SetSelectedCharacterDefinition(Definition);
}

FReply ULobbyWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();

    // Same key as the in-match pause menu, plus Start on a gamepad. Key events
    // bubble up from whichever slot or hero tile has focus, so this fires from
    // either page of the switcher
    if (Key == EKeys::P || Key == EKeys::Gamepad_Special_Right)
    {
        OpenLeaveConfirmation();
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void ULobbyWidget::OpenLeaveConfirmation()
{
    if (bIsLeaveMenuOpen || !GeneralMenuClass) { return; }

    // Note it before the dialog takes focus, not after
    CaptureFocusedSlot();

    GeneralMenuWidget = CreateWidget<UGeneralMenuWidget>(GetOwningPlayer(), GeneralMenuClass);
    if (!GeneralMenuWidget) { return; }

    bIsLeaveMenuOpen = true;
    GeneralMenuWidget->AddToViewport(200);

    // OpenMenu clears the delegate and hands it back, so the bind comes after
    GeneralMenuWidget->OpenMenu(EGeneralMenuType::YesNo,
        FText::FromString(TEXT("Are you sure you want to leave the match?\nIt will count as a loss on your record.")))
        .AddUObject(this, &ULobbyWidget::HandleLeaveMenuClosed);
}

void ULobbyWidget::HandleLeaveMenuClosed(bool bConfirmed)
{
    // The dialog removes itself on close either way
    bIsLeaveMenuOpen = false;
    GeneralMenuWidget = nullptr;

    if (!bConfirmed)
    {
        RestoreLobbyFocus();
        return;
    }

    // Mirrors AChrisPlayerController::HandleLeaveMatch so both routes land
    // on the multiplayer page rather than the main menu root
    if (UChrisGameInstance* GI = GetGameInstance<UChrisGameInstance>())
    {
        GI->bReturnToMultiplayerPage = true;
    }

    if (APlayerController* PC = GetOwningPlayer())
    {
        if (PC->PlayerCameraManager)
        {
            PC->PlayerCameraManager->StartCameraFade(0.f, 1.f, LeaveFadeDuration, FLinearColor::Black, false, true);
        }
    }

    GetWorld()->GetTimerManager().SetTimer(LeaveTravelTimerHandle, this,
        &ULobbyWidget::DoLeaveLobbyTravel, LeaveFadeDuration, false);
}

void ULobbyWidget::DoLeaveLobbyTravel()
{
    if (MainMenuLevel.IsNull()) { return; }

    const FString LevelName = FPackageName::ObjectPathToPackageName(MainMenuLevel.ToString());

    if (APlayerController* PC = GetOwningPlayer())
    {
        PC->ClientTravel(LevelName, ETravelType::TRAVEL_Absolute);
    }
}

// The slot's inner button holds the actual focus, not the slot widget itself,
// so a descendant check is needed as well as a direct one
void ULobbyWidget::CaptureFocusedSlot()
{
    FocusedSlotIndexBeforeMenu = INDEX_NONE;

    APlayerController* PC = GetOwningPlayer();
    if (!PC) { return; }

    for (int32 i = 0; i < TeamSelectionSlots.Num(); ++i)
    {
        UTeamSelectionWidget* InSlot = TeamSelectionSlots[i];
        if (!InSlot) { continue; }

        if (InSlot->HasUserFocus(PC) || InSlot->HasUserFocusedDescendants(PC))
        {
            FocusedSlotIndexBeforeMenu = i;
            return;
        }
    }
}

// Without this a controller player is left with focus nowhere after backing out,
// and the D-pad stops doing anything at all
void ULobbyWidget::RestoreLobbyFocus()
{
    // Deferred: the dialog removes itself as it closes, and setting focus in the same frame gets overridden as Slate tears it down
    GetWorld()->GetTimerManager().SetTimerForNextTick(
        FTimerDelegate::CreateWeakLambda(this, [this]()
            {
                const bool bOnTeamPage = MainSwitcher && MainSwitcher->GetActiveWidget() == TeamSelectionRoot;

                if (bOnTeamPage)
                {
                    // Fall back to the first slot if nothing was focused — better
                    const int32 Index = TeamSelectionSlots.IsValidIndex(FocusedSlotIndexBeforeMenu)
                        ? FocusedSlotIndexBeforeMenu : 0;

                    if (TeamSelectionSlots.IsValidIndex(Index) && TeamSelectionSlots[Index])
                    {
                        TeamSelectionSlots[Index]->FocusSlot();
                    }
                }
                else
                {
                    TryInitHeroSelectionFocus();
                }

                FocusedSlotIndexBeforeMenu = INDEX_NONE;
            }));
}