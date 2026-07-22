// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TeamSelectionWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSlotClicked, uint8 /*SlotID*/);

/**
 * Individual player slot in team selection. Handles click-to-select
 * and hover glow feedback. Broadcasts OnSlotClicked with its SlotID.
 */
UCLASS()
class UTeamSelectionWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetSlotID(uint8 NewSlotID);
	void UpdateSlotInfo(const FString& PlayerNickname);

	virtual void NativeConstruct() override;

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	FOnSlotClicked OnSlotClicked;

	void SetReadyVisual(bool bReady);

	// Focus drives the glow — fires for keyboard, gamepad, and mouse (via SetFocus)
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;

	// Focuses the inner button — for default slot highlighting from LobbyWidget
	void FocusSlot();

private:
	UPROPERTY(meta=(BindWidget))
	class UButton* SelectButton;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* InfoText;

	UPROPERTY(meta = (BindWidget))
	class UImage* HoverGlow;

	UFUNCTION()
	void SelectButtonClicked();

	uint8 SlotID;

	UPROPERTY(meta = (BindWidget))
	class UImage* PlayerBar;

	UPROPERTY(meta = (BindWidget))
	class UImage* GreenPlayerBar;

	
};
