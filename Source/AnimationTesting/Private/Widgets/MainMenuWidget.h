// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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

	/*****************************/
	/*          Main             */
	/*****************************/
private:
	UPROPERTY(meta = (BindWidget))
	class UWidget* LoginWidgetRoot;

	UPROPERTY(meta = (BindWidget))
	class UMenuButtonWidget* LoginButton;

	UFUNCTION()
	void LoginButtonClicked();

	void LoginCompleted(bool bWasSuccessful, const FString& PlayerNickname, const FString& ErrorMessage);
};
