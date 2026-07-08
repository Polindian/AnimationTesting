// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
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
};
