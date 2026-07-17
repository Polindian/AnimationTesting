// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Widgets/MenuButtonWidget.h"
#include "OnlineSessionSettings.h"
#include "MainMenuWidget.generated.h"


/**
 * 
 */
UCLASS()
class UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;


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

	/*****************************/
	/*          Waiting          */
	/*****************************/

private:
	UPROPERTY(meta = (BindWidget))
	class UWaitingWidget* WaitingWidget;

	FOnButtonClickedEvent& SwitchToWaitingWidget(const FText& WaitInfo, bool bAllowCancel = false);

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

	UPROPERTY(meta = (BindWidget))
	class UEditableText* NewSessionNameText;

	UFUNCTION()
	void CreateSessionButtonClicked();

	UFUNCTION()
	void NewSessionNameTextChanged(const FText& NewText);

	UFUNCTION()
	void CancelSessionCreation();

	void SwitchToMultiplayerPage();

	UPROPERTY(meta = (BindWidget))
	class USizeBox* SessionNameContainer;

	void JoinSessionFailed();
	void UpdateLobbyList(const TArray<FOnlineSessionSearchResult>& SearchResults);

	UPROPERTY(meta=(BindWidget))
	class UScrollBox* SessionScrollBox;

	UPROPERTY(EditDefaultsOnly, Category = "Session")
	TSubclassOf<class USessionEntryWidget> SessionEntryWidgetClass;

	FString CurrentSelectedSessionId = "";

	UPROPERTY(meta = (BindWidget))
	class UMenuButtonWidget* JoinSessionButton;

	UFUNCTION()
	void JoinSessionButtonClicked();

	void SessionEntrySelected(const FString& SelectedEntryIdString);

	UPROPERTY(meta = (BindWidget))
	class UMenuButtonWidget* TutorialBookButton;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial Book")
	TSubclassOf<class UTutorialBookWidget> TutorialBookClass;

	UPROPERTY()
	class UTutorialBookWidget* TutorialBook;

	UFUNCTION()
	void TutorialBookButtonClicked();
};
