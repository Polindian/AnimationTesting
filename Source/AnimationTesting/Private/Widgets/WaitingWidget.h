// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "WaitingWidget.generated.h"

/**
 * 
 */
UCLASS()
class UWaitingWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	FOnButtonClickedEvent& ClearAndGetButtonClickedEvent();

	void SetWaitInfoText(const FText& WaitInfo, bool bAllowCancel = false);

	void FocusCancelButton();
	
private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* WaitInfoText;

	UPROPERTY(meta=(BindWidget))
	UButton* CancelButton;


};
