// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyWidget.generated.h"

/**
 * Main lobby UI widget. Manages the team selection page
 * and dynamically populates player slots for each team.
 */
UCLASS()
class ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta=(BindWidget))
	class UWidgetSwitcher* MainSwitcher;

	UPROPERTY(meta=(BindWidget))
	class UWidget* TeamSelectionRoot;

	UPROPERTY(meta=(BindWidget))
	class UButton* ReadyUpButton;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* RedTeamBox;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* BlueTeamBox;

	UPROPERTY(EditDefaultsOnly, Category = "TeamSelection")
	TSubclassOf<class UTeamSelectionWidget> TeamSelectionWidgetClass;

	UPROPERTY()
	TArray<class UTeamSelectionWidget*> TeamSelectionSlots;

	void ClearAndPopulateTeamSelectionSlots();
	void SlotSelected(uint8 NewSlotId);
};
