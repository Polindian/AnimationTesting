// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Player/PlayerInfoTypes.h"
#include "ChrisGameState.generated.h"

class UPA_CharacterDefinition;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerSelectionUpdated, const TArray<FPlayerSelection>& /*NewPlayerSelection*/);

/**
 * 
 */
UCLASS()
class AChrisGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	void RequestPlayerSelectionChange(const APlayerState* RequestingPlayer, uint8 DesiredSlot);
	bool IsSlotOccupied(uint8 SlotId) const;

	void SetCharacterSelected(const APlayerState* SelectingPlayer, const UPA_CharacterDefinition* SelectedDefinition);
	void SetCharacterDeselected(const APlayerState* RequestingPlayer);
	void RequestPlayerLockIn(const APlayerState* RequestingPlayer, bool bLockIn);

	FOnPlayerSelectionUpdated OnPlayerSelectionUpdated;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	const TArray<FPlayerSelection>& GetPlayerSelection() const;

	void RequestPlayerReadyChange(const APlayerState* RequestingPlayer, bool bReady);

	// Returns true if all players are readied AND teams are balanced
	bool CanStartHeroSelection() const;

private:
	UPROPERTY(ReplicatedUsing = OnRep_PlayerSelectionArray)
	TArray<FPlayerSelection> PlayerSelectionArray;

	UFUNCTION()
	void OnRep_PlayerSelectionArray();
};
