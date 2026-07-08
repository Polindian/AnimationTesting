// Christopher Naglik All Rights Reserved


#include "Widgets/MainMenuWidget.h"
#include "Framework/ChrisGameInstance.h"
#include "Widgets/MenuButtonWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Widgets/WaitingWidget.h"

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


