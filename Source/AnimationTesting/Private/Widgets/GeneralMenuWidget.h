// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GeneralMenuWidget.generated.h"

UENUM()
enum class EGeneralMenuType : uint8
{
	YesNo,      // question — Yes and No in the bottom corners
	Continue    // notice — single CONTINUE button, bottom middle
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnGeneralMenuClosed, bool /*bConfirmed*/);

/**
 * Reusable dialog overlay. YesNo: bConfirmed = true on Yes, false on No or B.
 * Continue: always closes with bConfirmed = true (B also dismisses).
 * OpenMenu clears and returns the closed event — bind AFTER calling it,
 * same contract as the waiting widget.
 */
UCLASS()
class ANIMATIONTESTING_API UGeneralMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnGeneralMenuClosed& OpenMenu(EGeneralMenuType Type, const FText& Message);

protected:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* MessageText;

	UPROPERTY(meta = (BindWidget))
	class UMenuButtonWidget* YesButton;

	UPROPERTY(meta = (BindWidget))
	class UMenuButtonWidget* NoButton;

	UPROPERTY(meta = (BindWidget))
	class UMenuButtonWidget* ContinueButton;

private:
	EGeneralMenuType MenuType = EGeneralMenuType::Continue;
	FOnGeneralMenuClosed OnMenuClosed;

	UFUNCTION() void HandleYes();
	UFUNCTION() void HandleNo();
	UFUNCTION() void HandleContinue();

	void CloseMenu(bool bConfirmed);
};