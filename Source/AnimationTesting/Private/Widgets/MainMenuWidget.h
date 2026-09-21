// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Widgets/MenuButtonWidget.h"
#include "Widgets/GeneralMenuWidget.h"
#include "OnlineSessionSettings.h"
#include "MainMenuWidget.generated.h"

class UAudioComponent;

/**
 *
 */
UCLASS(Config = Game)
class UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	virtual void NativeDestruct() override;


	/*****************************/
	/*          Main             */
	/*****************************/

private:
	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* MainSwitcher;

	UPROPERTY()
	class UChrisGameInstance* ChrisGameInstance;

	void SwitchToMainWidget();

	UPROPERTY(meta = (BindWidget))
	class UWidget* MainWidgetRoot;

	/*****************************/
	/*          Login            */
	/*****************************/
private:
	UPROPERTY(meta = (BindWidget))
	class UWidget* LoginWidgetRoot;

	UPROPERTY(meta = (BindWidget))
	class UMenuButtonWidget* LoginButton;

	UFUNCTION()
	void LoginButtonClicked();

	void LoginCompleted(bool bWasSuccessful, const FString& PlayerNickname, const FString& ErrorMessage);

	// True on a cold launch until the intro hands over to a page. A login result
	// arriving before then is held, so it can't yank the player out of the intro
	bool bDeferLoginResult = false;

	// A failed login that arrived while deferred; shown once the login page has faded in
	bool bLoginErrorPending = false;

	// The login page is only a fallback now — decides what it shows based on
	// how far the automatic login got
	void ResolveLoginPage();

	void ShowLoginError();

	// DEBUG SKIP LOGIN — lands on the main page without logging in, so
	// coordinator-only features (leaderboard) can be tested before Steam login
	// works. Sessions won't work in this mode. Always off in Shipping builds.
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bDebugSkipLogin = false;

	bool ShouldSkipLogin() const;

	/*****************************/
	/*          Waiting          */
	/*****************************/

private:
	UPROPERTY(meta = (BindWidget))
	class UWaitingWidget* WaitingWidget;

	FOnMenuButtonClicked& SwitchToWaitingWidget(const FText& WaitInfo, bool bAllowCancel = false);

	void HideWaitingWidget();

	/*****************************/
	/*      Page Navigation      */
	/*****************************/

	// The main menu buttons
	UPROPERTY(meta = (BindWidget))
	class UMenuButtonWidget* StoryModeButton;

	UPROPERTY(meta = (BindWidget))
	class UMenuButtonWidget* MultiplayerButton;

	UPROPERTY(meta = (BindWidget))
	class UMenuButtonWidget* ExitGameButton;

	// Page roots inside MainSwitcher
	UPROPERTY(meta = (BindWidget))
	class UWidget* StoryModeRoot;

	UPROPERTY(meta = (BindWidget))
	class UWidget* MultiplayerPageRoot;

	// Back buttons living inside the pages
	UPROPERTY(meta = (BindWidget))
	class UMenuButtonWidget* StoryBackButton;

	UPROPERTY(meta = (BindWidget))
	class UMenuButtonWidget* MultiplayerBackButton;

	// Full-screen black image used for transitions; blocks clicks while fading
	UPROPERTY(meta = (BindWidget))
	class UImage* FadeImage;

	// BindWidgetAnim links these to animations of the same name in the WBP.
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* FadeOut;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* FadeIn;

	// The page we'll switch to once the screen is fully black
	UPROPERTY()
	UWidget* PendingPage;

	// One entry point for ALL page changes — the mini state machine
	void GoToPage(UWidget* TargetPage);


	FTimerHandle FadeHoldTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Transitions")
	float FadeHoldDuration = 0.35f;

	UFUNCTION() void HideFadeImage();

	UFUNCTION()
	void OnFadeHoldFinished();

	UFUNCTION()
	void OnFadeOutFinished();

	// Button handlers
	UFUNCTION()
	void StoryModeClicked();

	UFUNCTION()
	void MultiplayerClicked();

	UFUNCTION()
	void BackToMainClicked();

	UFUNCTION()
	void ExitGameClicked();

	/*****************************/
	/*      Multiplayer Page     */
	/*****************************/

private:
	UPROPERTY(meta = (BindWidget))
	class UMenuButtonWidget* CreateSessionButton;

	UFUNCTION()
	void CreateSessionButtonClicked();

	void NewSessionNameTextChanged(const FText& NewText);

	UFUNCTION()
	void CancelSessionCreation();

	UFUNCTION()
	void CancelSessionJoin();

	void SwitchToMultiplayerPage();

	UPROPERTY(meta = (BindWidget))
	class USizeBox* SessionNameContainer;



	UPROPERTY(meta = (BindWidget))
	class UScrollBox* SessionScrollBox;

	UPROPERTY(EditDefaultsOnly, Category = "Session")
	TSubclassOf<class USessionEntryWidget> SessionEntryWidgetClass;

	FString CurrentSelectedSessionId = "";

	UPROPERTY(meta = (BindWidget))
	class UMenuButtonWidget* JoinSessionButton;

	UFUNCTION()
	void JoinSessionButtonClicked();

	void SessionEntrySelected(const FString& SelectedEntryIdString);

	UPROPERTY(EditDefaultsOnly, Category = "Session")
	bool bDebugFillSessionList = false;

	void PopulateDebugSessionEntries();

	// X = inset from the scroll box sides, Y = gap above/below each entry
	UPROPERTY(EditDefaultsOnly, Category = "Session")
	FMargin SessionEntryPadding = FMargin(25.f, 12.f);

	UPROPERTY(meta = (BindWidget))
	class UMenuButtonWidget* TutorialBookButton;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial Book")
	TSubclassOf<class UTutorialBookWidget> TutorialBookClass;

	UPROPERTY()
	class UTutorialBookWidget* TutorialBook;

	UFUNCTION()
	void TutorialBookButtonClicked();

	void TutorialBookClosed();

	UPROPERTY(EditDefaultsOnly, Category = "General Menu")
	TSubclassOf<class UGeneralMenuWidget> GeneralMenuClass;

	UPROPERTY()
	class UGeneralMenuWidget* GeneralMenu;

	FOnGeneralMenuClosed& OpenGeneralMenu(EGeneralMenuType Type, const FText& Message);
	void SessionNameTooLong();

	// Substring match, so it catches padding and separators but also flags
	// innocent words that happen to contain a listed term
	bool ContainsProfanity(const FString& Name) const;
	void SessionNameProfane();

	UPROPERTY(EditDefaultsOnly, Category = "Session")
	TArray<FString> BlockedWords;

	UPROPERTY(meta = (BindWidget))
	class UMenuButtonWidget* PracticeArenaButton;

	UFUNCTION()
	void PracticeArenaClicked();

	UPROPERTY(Transient)
	UAudioComponent* WindAudio = nullptr;

	// Called whenever the active page changes, from any path
	void UpdatePageAmbience();

	UPROPERTY(EditDefaultsOnly, Category = "Multiplayer Page")
	TObjectPtr<class UMediaPlayer> BackgroundMediaPlayer;

	UPROPERTY(EditDefaultsOnly, Category = "Multiplayer Page")
	TObjectPtr<class UMediaSource> BackgroundMediaSource;

	bool bBackgroundVideoPlaying = false;

	UFUNCTION()
	void HandleBackgroundMediaOpened(FString OpenedUrl);

	void TravelFailed(const FString& Reason);
	void JoinSessionFailed();
	void UpdateLobbyList(const TArray<FOnlineSessionSearchResult>& SearchResults);


	/***********   Keyboard   *************/

	UPROPERTY(meta = (BindWidget))
	class USessionSearchBarWidget* SessionSearchBar;

	UPROPERTY(EditDefaultsOnly, Category = "Session")
	TSubclassOf<class UVirtualKeyboardWidget> VirtualKeyboardClass;

	UPROPERTY()
	class UVirtualKeyboardWidget* VirtualKeyboard;

	void OpenVirtualKeyboard();
	void VirtualKeyboardCommitted(const FText& FinalText);
	void VirtualKeyboardCancelled();

	// Re-run after every list repopulation — entry widgets are new objects each time
	void WireMultiplayerPageNavigation();

	void ResetCreateSessionFlow();


	/*****************************/
	/*          Settings         */
	/*****************************/

private:

	// The SETTINGS button on the main page
	UPROPERTY(meta = (BindWidget))
	class UMenuButtonWidget* SettingsButton;

	// WBP class to spawn 
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	TSubclassOf<class USettingsWidget> SettingsWidgetClass;

	// Kept so reopening reuses one instance instead of allocating each time
	UPROPERTY()
	class USettingsWidget* SettingsWidgetInstance;

	UFUNCTION()
	void SettingsClicked();

	void SettingsClosed();



	/*****************************/
	/*        Leaderboards       */
	/*****************************/

private:

	UPROPERTY(meta = (BindWidget))
	class UMenuButtonWidget* LeaderboardsButton;

	UPROPERTY(EditDefaultsOnly, Category = "Leaderboard")
	TSubclassOf<class ULeaderboardWidget> LeaderboardWidgetClass;

	UPROPERTY()
	class ULeaderboardWidget* LeaderboardInstance;

	UPROPERTY()
	class UMenuButtonWidget* LeaderboardOpener;

	UFUNCTION() void LeaderboardsClickedFromMain();
	UFUNCTION() void LeaderboardsClickedFromMultiplayer();

	void LeaderboardsClicked();

	void LeaderboardsClosed();

	// A second button on the multiplayer page
	UPROPERTY(meta = (BindWidget))
	class UMenuButtonWidget* MultiplayerLeaderboardsButton;


	/*****************************/
	/*        Game Title         */
	/*****************************/


	UPROPERTY(meta = (BindWidget))
	class UWidget* IntroRoot;

	UPROPERTY(EditDefaultsOnly, Category = "Intro")
	TObjectPtr<class UMediaPlayer> IntroMediaPlayer;

	UPROPERTY(EditDefaultsOnly, Category = "Intro")
	TObjectPtr<class UMediaSource> IntroMediaSource;

	UFUNCTION()
	void HandleIntroFinished();

	void PlayIntro();

	// OnEndReached never fires if the video fails to open — a missing file in a
	// packaged build, say. Without this the intro would lock on a black screen
	UPROPERTY(EditDefaultsOnly, Category = "Intro")
	float IntroMaxDuration = 25.f;

	FTimerHandle IntroSafetyTimerHandle;

	// How far into the intro the menu music starts, so it's already up when the
	// video ends rather than starting from silence
	UPROPERTY(EditDefaultsOnly, Category = "Intro")
	float MenuMusicStartDelay = 12.f;

	FTimerHandle MenuMusicStartTimerHandle;

	void StartMenuMusicNow();


	/*****************************/
	/*       Main Menu Video     */
	/*****************************/

	UPROPERTY(EditDefaultsOnly, Category = "Main Page")
	TObjectPtr<class UMediaPlayer> MainMenuMediaPlayer;

	UPROPERTY(EditDefaultsOnly, Category = "Main Page")
	TObjectPtr<class UMediaSource> MainMenuMediaSource;

	bool bMainMenuVideoPlaying = false;

	UFUNCTION()
	void HandleMainMenuMediaOpened(FString OpenedUrl);
};