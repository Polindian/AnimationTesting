// Christopher Naglik All Rights Reserved


#include "Widgets/MainMenuWidget.h"
#include "Framework/ChrisGameInstance.h"
#include "Widgets/MenuButtonWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Widgets/WaitingWidget.h"
#include "Components/Image.h"
#include "Animation/WidgetAnimation.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ChrisGameInstance = GetGameInstance<UChrisGameInstance>();
	if (ChrisGameInstance)
	{
		ChrisGameInstance->OnLoginCompleted.AddUObject(this, &UMainMenuWidget::LoginCompleted);
		// Widget may be created AFTER login already happened (GameInstance outlives UI) — sync to existing state instead of waiting for an event that already fired
		if (ChrisGameInstance->IsLoggedIn())
		{
			SwitchToMainWidget();
		}
	}

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

}

void UMainMenuWidget::StoryModeClicked() { GoToPage(StoryModeRoot); }
void UMainMenuWidget::MultiplayerClicked() { GoToPage(MultiplayerPageRoot); }
void UMainMenuWidget::BackToMainClicked() { GoToPage(MainWidgetRoot); }

void UMainMenuWidget::ExitGameClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, false);
}


void UMainMenuWidget::SwitchToMainWidget()
{
	if (MainSwitcher)
	{
		MainSwitcher->SetActiveWidget(MainWidgetRoot);
	}
}

// Widget trigger login UI
void UMainMenuWidget::LoginButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Logging in..."));	
	if (ChrisGameInstance && !ChrisGameInstance->IsLoggingIn() && !ChrisGameInstance->IsLoggedIn())
	{
		ChrisGameInstance->ClientAccountPortalLogin();
		SwitchToWaitingWidget(FText::FromString("LOGGING IN"));
	}
}

void UMainMenuWidget::LoginCompleted(bool bWasSuccessful, const FString& PlayerNickname, const FString& ErrorMessage)
{
	if (bWasSuccessful)
	{
		SwitchToMainWidget();
	}
	else
	{
		// Back to login page so the player can retry
		MainSwitcher->SetActiveWidget(LoginWidgetRoot);
	}
}

FOnButtonClickedEvent& UMainMenuWidget::SwitchToWaitingWidget(const FText& WaitInfo, bool bAllowCancel)
{
	MainSwitcher->SetActiveWidget(WaitingWidget);
	WaitingWidget->SetWaitInfoText(WaitInfo, bAllowCancel);
	return WaitingWidget->ClearAndGetButtonClickedEvent();
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
	PendingPage = nullptr;

	GetWorld()->GetTimerManager().SetTimer(
		FadeHoldTimerHandle, this, &UMainMenuWidget::OnFadeHoldFinished, FadeHoldDuration);
}

void UMainMenuWidget::OnFadeHoldFinished()
{
	PlayAnimation(FadeIn);
}

