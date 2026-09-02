// Christopher Naglik All Rights Reserved


#include "Widgets/MainMenuWidget.h"
#include "Framework/ChrisGameInstance.h"
#include "Widgets/MenuButtonWidget.h"
#include "Widgets/WaitingWidget.h"
#include "Widgets/SessionEntryWidget.h"
#include "Widgets/TutorialBookWidget.h"
#include "Widgets/SettingsWidget.h"
#include "Widgets/SessionSearchBarWidget.h"
#include "Widgets/VirtualKeyboardWidget.h"
#include "Widgets/GeneralMenuWidget.h"
#include "Widgets/LeaderboardWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Image.h"
#include "Components/EditableText.h"
#include "Components/SizeBox.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Network/ChrisNetStatics.h"
#include "Animation/WidgetAnimation.h"
#include "Audio/ChrisAudioSubsystem.h"
#include "Audio/ChrisGameplayTags.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MediaPlayer.h"
#include "MediaSource.h"
#include "Player/MainMenuPlayerController.h"



void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	HideWaitingWidget();

	ChrisGameInstance = GetGameInstance<UChrisGameInstance>();
	if (ChrisGameInstance)
	{
		ChrisGameInstance->OnLoginCompleted.AddUObject(this, &UMainMenuWidget::LoginCompleted);

		// Returning from a match — but only meaningful once logged in, since the multiplayer page needs a session. In PIE, solo runs skip login
		// entirely, so allow it through there for testing.
		bool bGoToMultiplayer = false;
		if (ChrisGameInstance->bReturnToMultiplayerPage)
		{
#if WITH_EDITOR
			bGoToMultiplayer = true;   // PIE: no login needed to inspect the page
#else
			bGoToMultiplayer = ChrisGameInstance->IsLoggedIn();
#endif
			ChrisGameInstance->bReturnToMultiplayerPage = false;
		}

		if (bGoToMultiplayer)
		{
			MainSwitcher->SetActiveWidget(MultiplayerPageRoot);
			ResetCreateSessionFlow();

			// Arrived from a match on a black screen — fade up into the page, matching how GoToPage transitions look
			FadeImage->SetVisibility(ESlateVisibility::Visible);
			PlayAnimation(FadeIn);
		}
		else if (ChrisGameInstance->IsLoggedIn())
		{
			SwitchToMainWidget();
		}
		ChrisGameInstance->OnJoinSessionFailed.AddUObject(this, &UMainMenuWidget::JoinSessionFailed);
		ChrisGameInstance->OnGlobalSessionSearchCompleted.AddUObject(this, &UMainMenuWidget::UpdateLobbyList);
		ChrisGameInstance->StartGlobalSessionSearch();
	}

	// Runs one frame later, when the Slate widgets actually exist
	GetWorld()->GetTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				// Explicit nav chain — spatial nav was skipping Multiplayer
				UWidget* Story = StoryModeButton->GetMainButton();
				UWidget* Multi = MultiplayerButton->GetMainButton();
				UWidget* Leader = LeaderboardsButton->GetMainButton();
				UWidget* Settings = SettingsButton->GetMainButton();
				UWidget* Exit = ExitGameButton->GetMainButton();

				Story->SetNavigationRuleExplicit(EUINavigation::Down, Multi);
				Story->SetNavigationRuleExplicit(EUINavigation::Up, Exit);
				Multi->SetNavigationRuleExplicit(EUINavigation::Up, Story);
				Multi->SetNavigationRuleExplicit(EUINavigation::Down, Leader);
				Leader->SetNavigationRuleExplicit(EUINavigation::Up, Multi);
				Leader->SetNavigationRuleExplicit(EUINavigation::Down, Settings);
				Settings->SetNavigationRuleExplicit(EUINavigation::Up, Leader);
				Settings->SetNavigationRuleExplicit(EUINavigation::Down, Exit);
				Exit->SetNavigationRuleExplicit(EUINavigation::Up, Settings);
				Exit->SetNavigationRuleExplicit(EUINavigation::Down, Story);

				for (UWidget* W : { Story, Multi, Leader, Settings, Exit })
				{
					W->BuildNavigation();
				}

				UWidget * ActivePage = MainSwitcher ? MainSwitcher->GetActiveWidget() : nullptr;

				if (ActivePage == MultiplayerPageRoot)
				{
					CreateSessionButton->FocusButton();
				}
				else if (ActivePage == MainWidgetRoot)
				{
					StoryModeButton->FocusButton();
				}
				else if (ActivePage == StoryModeRoot)
				{
					StoryBackButton->FocusButton();
				}
				else
				{
					LoginButton->FocusButton();
				}

				WireMultiplayerPageNavigation();
			}));

				LoginButton->OnMenuButtonClicked.AddDynamic(this, &UMainMenuWidget::LoginButtonClicked);

				// Every navigation button funnels into GoToPage — pages never switch themselves
				StoryModeButton->OnMenuButtonClicked.AddDynamic(this, &UMainMenuWidget::StoryModeClicked);
				MultiplayerButton->OnMenuButtonClicked.AddDynamic(this, &UMainMenuWidget::MultiplayerClicked);
				ExitGameButton->OnMenuButtonClicked.AddDynamic(this, &UMainMenuWidget::ExitGameClicked);

				LeaderboardsButton->OnMenuButtonClicked.AddDynamic(this, &UMainMenuWidget::LeaderboardsClickedFromMain);
				MultiplayerLeaderboardsButton->OnMenuButtonClicked.AddDynamic(this, &UMainMenuWidget::LeaderboardsClickedFromMultiplayer);

				PracticeArenaButton->OnMenuButtonClicked.AddDynamic(this, &UMainMenuWidget::PracticeArenaClicked);

				// Both pages' back buttons lead to the same place
				StoryBackButton->OnMenuButtonClicked.AddDynamic(this, &UMainMenuWidget::BackToMainClicked);
				MultiplayerBackButton->OnMenuButtonClicked.AddDynamic(this, &UMainMenuWidget::BackToMainClicked);

				// Bind animation-finished handlers ONCE — BindToAnimationFinished stacks duplicates if called repeatedly (e.g. per transition)
				FWidgetAnimationDynamicEvent FadeOutFinished;
				FadeOutFinished.BindDynamic(this, &UMainMenuWidget::OnFadeOutFinished);
				BindToAnimationFinished(FadeOut, FadeOutFinished);

				FWidgetAnimationDynamicEvent FadeInFinished;
				FadeInFinished.BindDynamic(this, &UMainMenuWidget::HideFadeImage);
				BindToAnimationFinished(FadeIn, FadeInFinished);

				CreateSessionButton->OnMenuButtonClicked.AddDynamic(this, &UMainMenuWidget::CreateSessionButtonClicked);
				SessionSearchBar->OnSearchTextChanged.AddUObject(this, &UMainMenuWidget::NewSessionNameTextChanged);
				SessionSearchBar->OnVirtualKeyboardRequested.AddUObject(this, &UMainMenuWidget::OpenVirtualKeyboard);
				SessionSearchBar->OnMaxLengthExceeded.AddUObject(this, &UMainMenuWidget::SessionNameTooLong);

				// Stage 1 state: button fully active, name field hidden until first press
				SessionNameContainer->SetVisibility(ESlateVisibility::Collapsed);

				JoinSessionButton->OnMenuButtonClicked.AddDynamic(this, &UMainMenuWidget::JoinSessionButtonClicked);
				JoinSessionButton->SetIsEnabled(false);

				TutorialBookButton->OnMenuButtonClicked.AddDynamic(this, &UMainMenuWidget::TutorialBookButtonClicked);

				SettingsButton->OnMenuButtonClicked.AddDynamic(this, &UMainMenuWidget::SettingsClicked);


				if (bDebugFillSessionList)
				{
					PopulateDebugSessionEntries();
				}

				if (BackgroundMediaPlayer)
				{
					BackgroundMediaPlayer->OnMediaOpened.AddDynamic(this, &UMainMenuWidget::HandleBackgroundMediaOpened);
				}

				if (MainMenuMediaPlayer)
				{
					MainMenuMediaPlayer->OnMediaOpened.AddDynamic(this, &UMainMenuWidget::HandleMainMenuMediaOpened);
				}

				UpdatePageAmbience();

				AMainMenuPlayerController* MenuPC = Cast<AMainMenuPlayerController>(GetOwningPlayer());

				// Not logged in means a cold launch — returning from a match keeps the login, so the intro doesn't replay
				if (ChrisGameInstance && !ChrisGameInstance->IsLoggedIn())
				{
					PlayIntro();
				}
				else if (MenuPC)
				{
					// No intro on this path, so nothing else will start the music
					StartMenuMusicNow();
				}
}

FReply UMainMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (Key == EKeys::BackSpace || Key == EKeys::Gamepad_FaceButton_Right)
	{
		// Waiting overlay up: B means cancel-the-operation, NEVER page navigation
		if (WaitingWidget->GetVisibility() != ESlateVisibility::Collapsed)
		{
			WaitingWidget->TriggerCancel();   // no-op when cancel isn't allowed (LOGGING IN)
			return FReply::Handled();
		}

		UWidget* ActivePage = MainSwitcher ? MainSwitcher->GetActiveWidget() : nullptr;
		if (ActivePage == StoryModeRoot || ActivePage == MultiplayerPageRoot)
		{
			GoToPage(MainWidgetRoot);
			return FReply::Handled();
		}
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UMainMenuWidget::NativeDestruct()
{
	UChrisAudioSubsystem::StopLoopingSound(WindAudio, 0.f);

	if (BackgroundMediaPlayer)
	{
		BackgroundMediaPlayer->OnMediaOpened.RemoveAll(this);
		BackgroundMediaPlayer->Close();
	}

	if (MainMenuMediaPlayer)
	{
		MainMenuMediaPlayer->OnMediaOpened.RemoveAll(this);
		MainMenuMediaPlayer->Close();
	}

	Super::NativeDestruct();
}

void UMainMenuWidget::StoryModeClicked() { GoToPage(StoryModeRoot); }
void UMainMenuWidget::MultiplayerClicked() { GoToPage(MultiplayerPageRoot); }
void UMainMenuWidget::BackToMainClicked() { GoToPage(MainWidgetRoot); }

void UMainMenuWidget::ExitGameClicked()
{
	OpenGeneralMenu(EGeneralMenuType::YesNo,
		NSLOCTEXT("MainMenu", "ConfirmExit", "ARE YOU SURE YOU WANT TO EXIT CHAMPIONS ARENA?"))
		.AddLambda([this](bool bConfirmed)
			{
				SetVisibility(ESlateVisibility::SelfHitTestInvisible);

				if (bConfirmed)
				{
					UKismetSystemLibrary::QuitGame(GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, false);
					return;
				}

				ExitGameButton->FocusButton();
			});
}

void UMainMenuWidget::CreateSessionButtonClicked()
{
		if (ChrisGameInstance && ChrisGameInstance->IsLoggedIn())
		{
			UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this);
			
			// Stage 1: field hidden -> this click only reveals the bar and arms stage 2
			if (SessionNameContainer->GetVisibility() == ESlateVisibility::Collapsed)
			{
				if (Audio) { Audio->Play2D(ChrisGameplayTags::Audio_UI_Lobby_TeamSlot); }

				SessionNameContainer->SetVisibility(ESlateVisibility::Visible);
				
				SessionNameContainer->SetVisibility(ESlateVisibility::Visible);
				SessionSearchBar->FocusBar();

				// Disable and dim until text arrives
				CreateSessionButton->SetIsEnabled(false);
				CreateSessionButton->SetRenderOpacity(0.5f);
				WireMultiplayerPageNavigation();
				return;
			}

			// Stage 2 (bar already visible): validate, then create
			if (SessionSearchBar->GetText().ToString().Len() > SessionSearchBar->GetMaxLength())
			{
				SessionNameTooLong();
				return;   // dialog instead of creating
			}

			if (Audio) { Audio->Play2D(ChrisGameplayTags::Audio_UI_Lobby_Continue); }

			ChrisGameInstance->RequestCreateAndJoinSession(FName(SessionSearchBar->GetText().ToString()));
			SwitchToWaitingWidget(FText::FromString("CREATING LOBBY"), true).AddDynamic(this, &UMainMenuWidget::CancelSessionCreation);
		}
}

void UMainMenuWidget::NewSessionNameTextChanged(const FText& NewText)
{
	const bool bHasName = !NewText.IsEmpty();
	CreateSessionButton->SetIsEnabled(bHasName);
	CreateSessionButton->SetRenderOpacity(bHasName ? 1.f : 0.5f);

	WireMultiplayerPageNavigation();
}

void UMainMenuWidget::CancelSessionCreation()
{
	if (ChrisGameInstance)
	{
		ChrisGameInstance->CancelSessionCreation();
	}

	HideWaitingWidget();
	SwitchToMultiplayerPage();

	GetWorld()->GetTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateWeakLambda(this, [this]() { CreateSessionButton->FocusButton(); }));
}

void UMainMenuWidget::SwitchToMultiplayerPage()
{
	if (MainSwitcher)
	{
		MainSwitcher->SetActiveWidget(MultiplayerPageRoot);
	}

	UpdatePageAmbience();
}

void UMainMenuWidget::JoinSessionFailed()
{
	SwitchToMultiplayerPage();
}

void UMainMenuWidget::UpdateLobbyList(const TArray<FOnlineSessionSearchResult>& SearchResults)
{
	UE_LOG(LogTemp, Warning, TEXT("Updating Session Search Results"));

	// Debug mode: the global search still fires and would ClearChildren the fake list
	if (bDebugFillSessionList) { return; }

	SessionScrollBox->ClearChildren();

	bool bCurrentSelectedSessionValid = false;
	for (const FOnlineSessionSearchResult& SearchResult : SearchResults)
	{
		USessionEntryWidget* NewSessionWidget = CreateWidget<USessionEntryWidget>(GetOwningPlayer(), SessionEntryWidgetClass);
		if (NewSessionWidget)
		{
			FString SessionName = "Name_None";
			SearchResult.Session.SessionSettings.Get<FString>(UChrisNetStatics::GetSessionNameKey(), SessionName);

			FString SessionIdString = SearchResult.Session.GetSessionIdStr();
			NewSessionWidget->InitializeEntry(SessionName, SessionIdString);
			NewSessionWidget->OnSessionEntrySelected.AddUObject(this, &UMainMenuWidget::SessionEntrySelected);

			SessionScrollBox->AddChild(NewSessionWidget);

			if (UScrollBoxSlot* BoxSlot = Cast<UScrollBoxSlot>(NewSessionWidget->Slot))
			{
				BoxSlot->SetPadding(SessionEntryPadding);
			}

			if (CurrentSelectedSessionId == SessionIdString)
			{
				bCurrentSelectedSessionValid = true;
			}
		}
	}

	CurrentSelectedSessionId = bCurrentSelectedSessionValid ? CurrentSelectedSessionId : "";
	JoinSessionButton->SetIsEnabled(bCurrentSelectedSessionValid);

	WireMultiplayerPageNavigation();
}

void UMainMenuWidget::JoinSessionButtonClicked()
{
	if (ChrisGameInstance && !CurrentSelectedSessionId.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Trying to join session with ID: %s"), *CurrentSelectedSessionId);
		if (ChrisGameInstance->JoinSessionWithId(CurrentSelectedSessionId))
		{
			SwitchToWaitingWidget(FText::FromString("JOINING SESSION"), false);
		}

	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot join session, no session selected"));
	}
}

void UMainMenuWidget::SessionEntrySelected(const FString& SelectedEntryIdString)
{
	CurrentSelectedSessionId = SelectedEntryIdString;
	JoinSessionButton->SetIsEnabled(true);

	// Exactly one entry carries the selected indicator
	for (UWidget* Child : SessionScrollBox->GetAllChildren())
	{
		if (USessionEntryWidget* Entry = Cast<USessionEntryWidget>(Child))
		{
			Entry->SetSelectedVisual(Entry->GetCachedSessionIdString() == SelectedEntryIdString);
		}
	}
}

void UMainMenuWidget::PopulateDebugSessionEntries()
{
	for (int32 i = 0; i < 10; ++i)
	{
		USessionEntryWidget* Entry = CreateWidget<USessionEntryWidget>(GetOwningPlayer(), SessionEntryWidgetClass);
		if (Entry)
		{
			Entry->InitializeEntry(FString::Printf(TEXT("Session Created %d"), i + 1),
				FString::Printf(TEXT("DebugId_%d"), i));
			Entry->OnSessionEntrySelected.AddUObject(this, &UMainMenuWidget::SessionEntrySelected);
			SessionScrollBox->AddChild(Entry);

			if (UScrollBoxSlot* BoxSlot = Cast<UScrollBoxSlot>(Entry->Slot))
			{
				BoxSlot->SetPadding(SessionEntryPadding);
			}
		}
	}

	WireMultiplayerPageNavigation();
}

void UMainMenuWidget::TutorialBookButtonClicked()
{
	if (!TutorialBook)
	{
		TutorialBook = CreateWidget<UTutorialBookWidget>(GetOwningPlayer(), TutorialBookClass);
		TutorialBook->OnBookClosed.AddUObject(this, &UMainMenuWidget::TutorialBookClosed);
	}
	TutorialBook->AddToViewport(10);
	SetVisibility(ESlateVisibility::HitTestInvisible);
	TutorialBook->OpenBook();
}

// Refocus the button which opened it
void UMainMenuWidget::TutorialBookClosed()
{
	TutorialBookButton->FocusButton();
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

FOnGeneralMenuClosed& UMainMenuWidget::OpenGeneralMenu(EGeneralMenuType Type, const FText& Message)
{
	if (!GeneralMenu)
	{
		GeneralMenu = CreateWidget<UGeneralMenuWidget>(GetOwningPlayer(), GeneralMenuClass);
	}
	GeneralMenu->AddToViewport(20);   // above everything, including other overlays
	SetVisibility(ESlateVisibility::HitTestInvisible);
	return GeneralMenu->OpenMenu(Type, Message);
}


void UMainMenuWidget::SessionNameTooLong()
{
	OpenGeneralMenu(EGeneralMenuType::Continue,
		FText::Format(NSLOCTEXT("MainMenu", "NameTooLong",
			"SESSION NAME CANNOT EXCEED {0} CHARACTERS"),
			FText::AsNumber(SessionSearchBar->GetMaxLength())))
		.AddLambda([this](bool)
			{
				SetVisibility(ESlateVisibility::SelfHitTestInvisible);
				SessionSearchBar->FocusBar();   // back to the highlighted bar to fix the name
			});
}

void UMainMenuWidget::PracticeArenaClicked()
{
	OpenGeneralMenu(EGeneralMenuType::YesNo,
		NSLOCTEXT("MainMenu", "ConfirmPracticeArena", "ARE YOU SURE YOU WANT TO ENTER THE PRACTICE ARENA?"))
		.AddLambda([this](bool bConfirmed)
			{
				SetVisibility(ESlateVisibility::SelfHitTestInvisible);

				if (bConfirmed)
				{
					if (ChrisGameInstance) { ChrisGameInstance->StartPracticeArena(); }
					return;
				}

				PracticeArenaButton->FocusButton();
			});
}

void UMainMenuWidget::UpdatePageAmbience()
{
	const bool bOnMultiplayerPage = (MainSwitcher && MainSwitcher->GetActiveWidget() == MultiplayerPageRoot);

	if (bOnMultiplayerPage && !WindAudio)
	{
		if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
		{
			WindAudio = Audio->PlayLooping2DFadeIn(ChrisGameplayTags::Audio_Ambience_MultiplayerWind, 2.f);
		}
	}
	else if (!bOnMultiplayerPage && WindAudio)
	{
		UChrisAudioSubsystem::StopLoopingSound(WindAudio, 2.f);
	}

	// Video decoding is expensive, so it only runs while the page is actually up
	if (BackgroundMediaPlayer && BackgroundMediaSource)
	{
		if (bOnMultiplayerPage && !bBackgroundVideoPlaying)
		{
			BackgroundMediaPlayer->SetNativeVolume(0.f);
			BackgroundMediaPlayer->OpenSource(BackgroundMediaSource);
			bBackgroundVideoPlaying = true;
		}
		else if (!bOnMultiplayerPage && bBackgroundVideoPlaying)
		{
			BackgroundMediaPlayer->Close();
			bBackgroundVideoPlaying = false;
		}
	}

	const bool bOnMainPage = MainSwitcher && MainSwitcher->GetActiveWidget() == MainWidgetRoot;

	// Same as the multiplayer video: only decode while its page is up
	if (MainMenuMediaPlayer && MainMenuMediaSource)
	{
		if (bOnMainPage && !bMainMenuVideoPlaying)
		{
			MainMenuMediaPlayer->SetNativeVolume(0.f);
			MainMenuMediaPlayer->OpenSource(MainMenuMediaSource);
			bMainMenuVideoPlaying = true;
		}
		else if (!bOnMainPage && bMainMenuVideoPlaying)
		{
			MainMenuMediaPlayer->Close();
			bMainMenuVideoPlaying = false;
		}
	}
}

void UMainMenuWidget::HandleBackgroundMediaOpened(FString OpenedUrl)
{
	if (!BackgroundMediaPlayer) { return; }

	UE_LOG(LogTemp, Warning, TEXT("[MultiplayerBG] Media opened: %s"), *OpenedUrl);

	BackgroundMediaPlayer->SetLooping(true);
	BackgroundMediaPlayer->Play();
}

void UMainMenuWidget::OpenVirtualKeyboard()
{
	if (!VirtualKeyboardClass) { return; }

	if (!VirtualKeyboard)
	{
		VirtualKeyboard = CreateWidget<UVirtualKeyboardWidget>(GetOwningPlayer(), VirtualKeyboardClass);
		VirtualKeyboard->OnCommitted.AddUObject(this, &UMainMenuWidget::VirtualKeyboardCommitted);
		VirtualKeyboard->OnCancelled.AddUObject(this, &UMainMenuWidget::VirtualKeyboardCancelled);
	}

	VirtualKeyboard->AddToViewport(10);
	SetVisibility(ESlateVisibility::HitTestInvisible);   // menu dead while OSK is up
	VirtualKeyboard->OpenKeyboard(SessionSearchBar->GetText());
}

void UMainMenuWidget::VirtualKeyboardCommitted(const FText& FinalText)
{
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	SessionSearchBar->SetTextFromKeyboard(FinalText);

	if (FinalText.ToString().Len() > SessionSearchBar->GetMaxLength())
	{
		SessionNameTooLong();
	} 
}

void UMainMenuWidget::VirtualKeyboardCancelled()
{
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	SessionSearchBar->FocusBar();
}

// Re-run whenever anything it depends on changes: list repopulated, bar
// revealed/hidden, name text changed. One function owns the whole page's nav.
void UMainMenuWidget::WireMultiplayerPageNavigation()
{
	USessionEntryWidget* FirstEntry = nullptr;

	for (UWidget* Child : SessionScrollBox->GetAllChildren())
	{
		FirstEntry = Cast<USessionEntryWidget>(Child);
		if (FirstEntry) { break; }
	}

	UWidget* FirstEntryButton = FirstEntry ? FirstEntry->GetSessionButton() : nullptr;
	UWidget* CreateBtn = CreateSessionButton->GetMainButton();
	UWidget* BarBtn = SessionSearchBar->GetBarButton();
	UWidget* TutorialBtn = TutorialBookButton->GetMainButton();
	UWidget* LeaderboardsBtn = MultiplayerLeaderboardsButton->GetMainButton();
	UWidget* PracticeBtn = PracticeArenaButton->GetMainButton();
	UWidget* ReturnBtn = MultiplayerBackButton->GetMainButton();

	const bool bBarVisible = SessionNameContainer->GetVisibility() != ESlateVisibility::Collapsed;
	const bool bHasName = !SessionSearchBar->GetText().IsEmpty();
	// Disabled exactly when the bar is up with an empty name — and explicit nav
	// rules don't skip disabled widgets, so no rule may target it while disabled
	const bool bCreateEnabled = !bBarVisible || bHasName;

	// Right from CreateLobby: the bar when it's up, otherwise straight to the list
	if (bBarVisible)
	{
		CreateBtn->SetNavigationRuleExplicit(EUINavigation::Right, BarBtn);
	}
	else if (FirstEntryButton)
	{
		CreateBtn->SetNavigationRuleExplicit(EUINavigation::Right, FirstEntryButton);
	}
	else
	{
		CreateBtn->SetNavigationRuleBase(EUINavigation::Right, EUINavigationRule::Stop);
	}

	// TutorialBook can also hop straight into the list
	if (FirstEntryButton)
	{
		TutorialBtn->SetNavigationRuleExplicit(EUINavigation::Right, FirstEntryButton);
		LeaderboardsBtn->SetNavigationRuleExplicit(EUINavigation::Right, FirstEntryButton);
		PracticeBtn->SetNavigationRuleExplicit(EUINavigation::Right, FirstEntryButton);
	}

	// Left from the bar: only when CreateLobby is a legal (enabled) target
	if (bCreateEnabled)
	{
		BarBtn->SetNavigationRuleExplicit(EUINavigation::Left, CreateBtn);
	}
	else
	{
		BarBtn->SetNavigationRuleBase(EUINavigation::Left, EUINavigationRule::Stop);
	}

	// Right from the bar into the list
	if (FirstEntryButton)
	{
		BarBtn->SetNavigationRuleExplicit(EUINavigation::Right, FirstEntryButton);
	}

	// Left from any entry: to the bar when it's up, otherwise to CreateLobby
	UWidget* ListLeftTarget = bBarVisible ? BarBtn : CreateBtn;
	for (UWidget* Child : SessionScrollBox->GetAllChildren())
	{
		if (USessionEntryWidget* Entry = Cast<USessionEntryWidget>(Child))
		{
			Entry->GetSessionButton()->SetNavigationRuleExplicit(EUINavigation::Left, ListLeftTarget);
			Entry->GetSessionButton()->BuildNavigation();
		}
	}

	// TutorialBook sits below CreateLobby — its Up hop needs the same gate
	if (bCreateEnabled)
	{
		TutorialBtn->SetNavigationRuleExplicit(EUINavigation::Up, CreateBtn);
	}
	else
	{
		TutorialBtn->SetNavigationRuleBase(EUINavigation::Up, EUINavigationRule::Stop);
	}

	// LeaderboardsButton underneath TutorialBook — both directions
	LeaderboardsBtn->SetNavigationRuleExplicit(EUINavigation::Up, TutorialBtn);
	LeaderboardsBtn->SetNavigationRuleExplicit(EUINavigation::Down, PracticeBtn);
	PracticeBtn->SetNavigationRuleExplicit(EUINavigation::Up, LeaderboardsBtn);
	PracticeBtn->SetNavigationRuleExplicit(EUINavigation::Down, ReturnBtn);


	CreateBtn->BuildNavigation();
	BarBtn->BuildNavigation();
	TutorialBtn->BuildNavigation();
	LeaderboardsBtn->BuildNavigation();
	PracticeBtn->BuildNavigation();
	ReturnBtn->BuildNavigation();
}

void UMainMenuWidget::ResetCreateSessionFlow()
{
	SessionSearchBar->ClearText();
	SessionNameContainer->SetVisibility(ESlateVisibility::Collapsed);
	CreateSessionButton->SetIsEnabled(true);
	CreateSessionButton->SetRenderOpacity(1.f);
	WireMultiplayerPageNavigation();
}



void UMainMenuWidget::SettingsClicked()
{
	if (!SettingsWidgetClass) { return; }

	// Create once, reuse afterwards — OpenSettings resets state each time
	if (!SettingsWidgetInstance)
	{
		SettingsWidgetInstance = CreateWidget<USettingsWidget>(GetOwningPlayer(), SettingsWidgetClass);
		// Restore controller focus to the menu when settings closes
		SettingsWidgetInstance->OnSettingsClosed.AddUObject(this, &UMainMenuWidget::SettingsClosed);
	}

	if (SettingsWidgetInstance && !SettingsWidgetInstance->IsInViewport())
	{
		// High ZOrder so it draws over the menu (and the fade image)
		SettingsWidgetInstance->AddToViewport(10);
		SetVisibility(ESlateVisibility::HitTestInvisible);
		SettingsWidgetInstance->OpenSettings();
	}
}

void UMainMenuWidget::SettingsClosed()
{
	// Back to a sensible default so gamepad navigation isn't stranded
	SettingsButton->FocusButton();
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UMainMenuWidget::LeaderboardsClicked()
{
	if (!LeaderboardWidgetClass) return;

	if (!LeaderboardInstance)
	{
		LeaderboardInstance = CreateWidget<ULeaderboardWidget>(GetOwningPlayer(), LeaderboardWidgetClass);
		LeaderboardInstance->OnLeaderboardClosed.AddUObject(this, &UMainMenuWidget::LeaderboardsClosed);
	}

	if (!LeaderboardInstance->IsInViewport())
	{
		LeaderboardInstance->AddToViewport(10);
		SetVisibility(ESlateVisibility::HitTestInvisible);   // menu dead underneath
		LeaderboardInstance->OpenLeaderboard();
	}
}

void UMainMenuWidget::LeaderboardsClickedFromMain()
{
	LeaderboardOpener = LeaderboardsButton;
	LeaderboardsClicked();
}

void UMainMenuWidget::LeaderboardsClickedFromMultiplayer()
{
	LeaderboardOpener = MultiplayerLeaderboardsButton;
	LeaderboardsClicked();
}

void UMainMenuWidget::LeaderboardsClosed()
{
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (LeaderboardOpener) { LeaderboardOpener->FocusButton(); }
}

void UMainMenuWidget::PlayIntro()
{
	if (!IntroMediaPlayer || !IntroMediaSource)
	{
		// No video set — don't strand the player on a black screen
		GoToPage(LoginWidgetRoot);
		return;
	}

	MainSwitcher->SetActiveWidget(IntroRoot);

	IntroMediaPlayer->OnEndReached.AddDynamic(this, &UMainMenuWidget::HandleIntroFinished);
	IntroMediaPlayer->OpenSource(IntroMediaSource);

	// Set slightly longer than the video, so it only fires if playback failed
	GetWorld()->GetTimerManager().SetTimer(IntroSafetyTimerHandle, this,
		&UMainMenuWidget::HandleIntroFinished, IntroMaxDuration, false);

	// Comes up under the back half of the video, so the transition to the login
	// page isn't a jump from video audio straight into silence
	GetWorld()->GetTimerManager().SetTimer(MenuMusicStartTimerHandle, this,
		&UMainMenuWidget::StartMenuMusicNow, MenuMusicStartDelay, false);

	// Opened during the intro so the first frame is decoded before the main page
	// is ever shown — otherwise there's a visible black gap on arrival
	if (MainMenuMediaPlayer && MainMenuMediaSource && !bMainMenuVideoPlaying)
	{
		MainMenuMediaPlayer->SetNativeVolume(0.f);
		MainMenuMediaPlayer->OpenSource(MainMenuMediaSource);
		bMainMenuVideoPlaying = true;
	}
}

void UMainMenuWidget::StartMenuMusicNow()
{
	if (AMainMenuPlayerController* MenuPC = Cast<AMainMenuPlayerController>(GetOwningPlayer()))
	{
		MenuPC->StartMenuMusic();
	}
}
void UMainMenuWidget::HandleMainMenuMediaOpened(FString OpenedUrl)
{
	UE_LOG(LogTemp, Warning, TEXT("[MainMenuBG] Media opened: %s"), *OpenedUrl);
	
	if (!MainMenuMediaPlayer) { return; }

	MainMenuMediaPlayer->SetLooping(true);
	MainMenuMediaPlayer->Play();
}

void UMainMenuWidget::HandleIntroFinished()
{
	// Called from either the media callback or the safety timer, so it has to be safe to reach twice — clear both, then move on
	GetWorld()->GetTimerManager().ClearTimer(IntroSafetyTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(MenuMusicStartTimerHandle);

	// Covers a video that ended early — the delay timer wouldn't have fired yet
	StartMenuMusicNow();

	if (IntroMediaPlayer)
	{
		IntroMediaPlayer->OnEndReached.RemoveDynamic(this, &UMainMenuWidget::HandleIntroFinished);
		IntroMediaPlayer->Close();
	}

	if (AMainMenuPlayerController* PC = Cast<AMainMenuPlayerController>(GetOwningPlayer()))
	{
		PC->StartMenuMusic();
	}

	// Reuses the existing fade-to-black-and-swap path
	GoToPage(LoginWidgetRoot);
}

void UMainMenuWidget::SwitchToMainWidget()
{
	if (MainSwitcher)
	{
		MainSwitcher->SetActiveWidget(MainWidgetRoot);
		StoryModeButton->FocusButton();

		UpdatePageAmbience();
	}
}

// Widget trigger login UI
void UMainMenuWidget::LoginButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Logging in..."));	
	if (ChrisGameInstance && !ChrisGameInstance->IsLoggingIn() && !ChrisGameInstance->IsLoggedIn())
	{
		// -DevAuthCred=Name on the command line -> DevAuth tool; absent -> normal Account Portal
		FString DevAuthCred;
		if (FParse::Value(FCommandLine::Get(), TEXT("DevAuthCred="), DevAuthCred))
		{
			ChrisGameInstance->ClientDevAuthLogin(DevAuthCred);
		}
		else
		{
			ChrisGameInstance->ClientAccountPortalLogin();
		}
		SwitchToWaitingWidget(FText::FromString("LOGGING IN"));
	}
}

void UMainMenuWidget::LoginCompleted(bool bWasSuccessful, const FString& PlayerNickname, const FString& ErrorMessage)
{
	HideWaitingWidget();
	
	if (bWasSuccessful)
	{
		SwitchToMainWidget();
	}
	else
	{
		// Back to login page so the player can retry
		MainSwitcher->SetActiveWidget(LoginWidgetRoot);

		LoginButton->FocusButton();
	}
}

FOnMenuButtonClicked& UMainMenuWidget::SwitchToWaitingWidget(const FText& WaitInfo, bool bAllowCancel)
{
	// Overlay ON TOP of the current page (which keeps rendering, blurred) — not a switcher page anymore
	WaitingWidget->SetVisibility(ESlateVisibility::Visible);
	WaitingWidget->SetWaitInfoText(WaitInfo, bAllowCancel);

	if (bAllowCancel)
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateWeakLambda(this, [this]() { WaitingWidget->FocusCancelButton(); }));
	}

	return WaitingWidget->ClearAndGetButtonClickedEvent();
}

void UMainMenuWidget::HideWaitingWidget()
{
	WaitingWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void UMainMenuWidget::GoToPage(UWidget* TargetPage)
{
	if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
	{
		Audio->Play2D(ChrisGameplayTags::Audio_UI_Page_Change);
	}
	
	if (!TargetPage || PendingPage) return;

	PendingPage = TargetPage;
	FadeImage->SetVisibility(ESlateVisibility::Visible);
	PlayAnimation(FadeOut);  
}

// FadeIn finished — image is fully transparent; hide it so it stops intercepting mouse clicks
void UMainMenuWidget::HideFadeImage()
{
	FadeImage->SetVisibility(ESlateVisibility::Hidden);
}

void UMainMenuWidget::OnFadeOutFinished()
{
	// Guard: only act if a transition is actually pending
	if (!PendingPage) return;

	MainSwitcher->SetActiveWidget(PendingPage);

	// Console-style default: top button of the new page starts focused/highlighted
	if (PendingPage == MainWidgetRoot)
	{
		StoryModeButton->FocusButton();
	}
	else if (PendingPage == MultiplayerPageRoot)
	{
		ResetCreateSessionFlow();
		CreateSessionButton->FocusButton();
	}
	else if (PendingPage == StoryModeRoot)
	{
		StoryBackButton->FocusButton();
	}
	else if (PendingPage == LoginWidgetRoot)
	{
		LoginButton->FocusButton();
	}

	PendingPage = nullptr;

	UpdatePageAmbience();

	GetWorld()->GetTimerManager().SetTimer(
		FadeHoldTimerHandle, this, &UMainMenuWidget::OnFadeHoldFinished, FadeHoldDuration);
}


void UMainMenuWidget::OnFadeHoldFinished()
{
	PlayAnimation(FadeIn);
}

