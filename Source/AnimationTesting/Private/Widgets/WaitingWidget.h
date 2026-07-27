#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/MenuButtonWidget.h"  
#include "WaitingWidget.generated.h"

class UBackHintWidget;

UCLASS()
class UWaitingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnMenuButtonClicked& ClearAndGetButtonClickedEvent();   // return type changed

	void SetWaitInfoText(const FText& WaitInfo, bool bInAllowCancel = false);
	void FocusCancelButton();
	void TriggerCancel();

private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* WaitInfoText;

	UPROPERTY(meta = (BindWidget))
	class UMenuButtonWidget* CancelButton;   

	// Backspace/B hint — only shown when cancelling is allowed
	UPROPERTY(meta = (BindWidget))
	UBackHintWidget* BackHint;

	bool bAllowCancel = false;
};