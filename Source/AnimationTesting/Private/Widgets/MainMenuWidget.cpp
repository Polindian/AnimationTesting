// Christopher Naglik All Rights Reserved


#include "Widgets/MainMenuWidget.h"
#include "Components/Button.h"
#include "Framework/ChrisGameInstance.h"
#include "Widgets/MenuButtonWidget.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ChrisGameInstance = GetGameInstance<UChrisGameInstance>();
	if (ChrisGameInstance)
	{

	}

	LoginButton->OnMenuButtonClicked.AddDynamic(this, &UMainMenuWidget::LoginButtonClicked);
}

void UMainMenuWidget::LoginButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Logging in..."));	
}

void UMainMenuWidget::LoginCompleted(bool bWasSuccessful, const FString& PlayerNickname, const FString& ErrorMessage)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("Login Successful!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Login Failed"));
	}

}


