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
#include "Components/WidgetSwitcher.h"
#include "Components/Image.h"
#include "Components/EditableText.h"
#include "Components/SizeBox.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Network/ChrisNetStatics.h"
#include "Animation/WidgetAnimation.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	HideWaitingWidget();

	ChrisGameInstance = GetGameInstance<UChrisGameInstance>();
	if (ChrisGameInstance)
	{
		ChrisGameInstance->OnLoginCompleted.AddUObject(this, &UMainMenuWidget::LoginCompleted);
		// Widget may be created AFTER login already happened (GameInstance outlives UI) — sync to existing state instead of waiting for an event that already fired
		if (ChrisGameInstance->IsLoggedIn())
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

				// Initial focus — login button, or main page default if already logged in
				if (ChrisGameInstance && ChrisGameInstance->IsLoggedIn())
				{
					StoryModeButton->FocusButton();
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

				// Stage 1 state: button fully active, name field hidden until first press
				SessionNameContainer->SetVisibility(ESlateVisibility::Collapsed);

				JoinSessionButton->OnMenuButtonClicked.AddDynamic(this, &UMainMenuWidget::JoinSessionButtonClicked);
				JoinSessionButton->SetIsEnabled(false);

				TutorialBookButton->OnMenuButtonClicked.AddDynamic(this, &UMainMenuWidget::TutorialBookButtonClicked);

				SettingsButton->OnMenuButtonClicked.AddDynamic(this, &UMainMenuWidget::SettingsClicked);

				if (!bDebugFillSessionList) return;

				if (bDebugFillSessionList)
				{
					PopulateDebugSessionEntries();
				}
}

FReply UMainMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (Key == EKeys::BackSpace || Key == EKeys::Gamepad_FaceButton_Right)
	{
		// Back only makes sense from the sub-pages — main and login have nowhere to go
		UWidget* ActivePage = MainSwitcher ? MainSwitcher->GetActiveWidget() : nullptr;
		if (ActivePage == StoryModeRoot || ActivePage == MultiplayerPageRoot)
		{
			GoToPage(MainWidgetRoot);
			return FReply::Handled();
		}
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UMainMenuWidget::StoryModeClicked() { GoToPage(StoryModeRoot); }
void UMainMenuWidget::MultiplayerClicked() { GoToPage(MultiplayerPageRoot); }
void UMainMenuWidget::BackToMainClicked() { GoToPage(MainWidgetRoot); }

void UMainMenuWidget::ExitGameClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UMainMenuWidget::CreateSessionButtonClicked()
{
	if (ChrisGameInstance && ChrisGameInstance->IsLoggedIn())
	{
		// Stage 1: field hidden -> click reveals bar and arms stage 2
		if (SessionNameContainer->GetVisibility() == ESlateVisibility::Collapsed)
		{
			SessionNameContainer->SetVisibility(ESlateVisibility::Visible);
			SessionSearchBar->FocusBar();

			// Disable and dim until text arrives
			CreateSessionButton->SetIsEnabled(false);
			CreateSessionButton->SetRenderOpacity(0.5f);
			WireMultiplayerPageNavigation();
			return;
		}

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
}

void UMainMenuWidget::SwitchToMultiplayerPage()
{
	if (MainSwitcher)
	{
		MainSwitcher->SetActiveWidget(MultiplayerPageRoot);
	}
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
	SessionSearchBar->SetTextFromKeyboard(FinalText);   // sets text, enables CreateLobby, refocuses the bar
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

	CreateBtn->BuildNavigation();
	BarBtn->BuildNavigation();
	TutorialBtn->BuildNavigation();
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


void UMainMenuWidget::SwitchToMainWidget()
{
	if (MainSwitcher)
	{
		MainSwitcher->SetActiveWidget(MainWidgetRoot);
		StoryModeButton->FocusButton();
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

FOnButtonClickedEvent& UMainMenuWidget::SwitchToWaitingWidget(const FText& WaitInfo, bool bAllowCancel)
{
	// Overlay ON TOP of the current page (which keeps rendering, blurred) — not a switcher page anymore
	WaitingWidget->SetVisibility(ESlateVisibility::Visible);
	WaitingWidget->SetWaitInfoText(WaitInfo, bAllowCancel);

	if (bAllowCancel)
	{
		WaitingWidget->FocusCancelButton();
	}

	return WaitingWidget->ClearAndGetButtonClickedEvent();
}

void UMainMenuWidget::HideWaitingWidget()
{
	WaitingWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void UMainMenuWidget::GoToPage(UWidget* TargetPage)
{
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
	if (PendingPage == MainWidgetRoot) { StoryModeButton->FocusButton(); }
	else if (PendingPage == MultiplayerPageRoot) { CreateSessionButton->FocusButton(); }
	else if (PendingPage == StoryModeRoot) { StoryBackButton->FocusButton(); }
	else if (PendingPage == MultiplayerPageRoot)
	{
		ResetCreateSessionFlow();
		CreateSessionButton->FocusButton();
	}

	PendingPage = nullptr;

	GetWorld()->GetTimerManager().SetTimer(
		FadeHoldTimerHandle, this, &UMainMenuWidget::OnFadeHoldFinished, FadeHoldDuration);
}

void UMainMenuWidget::OnFadeHoldFinished()
{
	PlayAnimation(FadeIn);
}

