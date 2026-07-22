// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/PlayerInfoTypes.h"
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
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UPROPERTY(meta=(BindWidget))
	class UWidgetSwitcher* MainSwitcher;

	UPROPERTY(meta=(BindWidget))
	class UWidget* TeamSelectionRoot;

	UPROPERTY(meta=(BindWidget))
	class UMenuButtonWidget* ReadyUpButton;

	bool bIsReady = false;

	UFUNCTION()
	void OnReadyUpClicked();

	void SetReadyState(bool bReady);

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* RedTeamBox;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* BlueTeamBox;

	// Player Selection Logic

	UPROPERTY(EditDefaultsOnly, Category = "TeamSelection")
	TSubclassOf<class UTeamSelectionWidget> TeamSelectionWidgetClass;

	UPROPERTY()
	TArray<class UTeamSelectionWidget*> TeamSelectionSlots;

	void ClearAndPopulateTeamSelectionSlots();
	void SlotSelected(uint8 NewSlotId);

	UPROPERTY()
	class ALobbyPlayerController* LobbyPlayerController;

	UPROPERTY()
	class AChrisPlayerState* ChrisPlayerState;

	FTimerHandle ConfigureGameStateTimerHandle;
	void ConfigureGameState();

	UPROPERTY()
	class AChrisGameState* ChrisGameState;

	void UpdatePlayerSelectionDisplay(const TArray<FPlayerSelection>& PlayerSelection);

	// Called by the controller's delegate when server triggers page switch
	void SwitchToHeroSelection();


	// Callback fired by the Asset Manager once all CharacterDefinition
	// assets have finished async loading — this is where we'll populate
	// the hero selection UI with icons, names, and preview data
	void CharacterDefinitionsLoaded();

	// Second page in MainSwitcher — hero selection
	UPROPERTY(meta = (BindWidget))
	class UWidget* HeroSelectionRoot;

	// Second page in MainSwitcher — hero selection
	UPROPERTY(meta = (BindWidget))
	class UTileView* CharacterSelectionTileView;

	void CharacterSelected(UObject* SelectedUObject);


	UPROPERTY(EditDefaultsOnly, Category = "Character Display")
	TSubclassOf<class ACharacterDisplay> CharacterDisplayClass;

	UPROPERTY()
	class ACharacterDisplay* CharacterDisplay;

	UPROPERTY(meta=(BindWidget))
	class UPlayerTeamLayoutWidget* PlayerTeamLayoutWidget;

	void SpawnCharacterDisplay();
	void UpdateCharacterDisplay(const FPlayerSelection& PlayerSelection);

	UPROPERTY(meta = (BindWidget))
	class UMenuButtonWidget* StartMatchButton;

	// Local tracking of whether this client is locked in
	bool bIsLockedIn = false;

	UFUNCTION()
	void OnStartMatchButtonClicked();

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HeroSelectionTimerText;

	UPROPERTY()
	const UPA_CharacterDefinition* CurrentDisplayedDefinition = nullptr;
};
