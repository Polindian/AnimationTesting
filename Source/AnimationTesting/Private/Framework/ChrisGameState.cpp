// Christopher Naglik All Rights Reserved


#include "Framework/ChrisGameState.h"
#include "Net/UnrealNetwork.h"

void AChrisGameState::RequestPlayerSelectionChange(const APlayerState* RequestingPlayer, uint8 DesiredSlot)
{
	if(!HasAuthority() || IsSlotOccupied(DesiredSlot))
	{
		return;
	}

	// Check if this player already has an entry (they're switching slots, not picking for the first time)
	FPlayerSelection* PlayerSelectionPtr = PlayerSelectionArray.FindByPredicate([&] (const FPlayerSelection& PlayerSelection)
	{
		return PlayerSelection.IsForPlayer(RequestingPlayer);
		});

	if (PlayerSelectionPtr)
	{
		PlayerSelectionPtr->SetSlot(DesiredSlot);
	}
	else
	{
		PlayerSelectionArray.Add(FPlayerSelection(DesiredSlot, RequestingPlayer));
	}

	ForceNetUpdate();
	OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
}

bool AChrisGameState::IsSlotOccupied(uint8 SlotId) const
{
	for(const FPlayerSelection& PlayerSelection : PlayerSelectionArray)
	{
		if(PlayerSelection.GetPlayerSlot() == SlotId)
		{
			return true;
		}
	}

	return false;
}

// Registers PlayerSelectionArray for replication — fires OnRep on every change
void AChrisGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(AChrisGameState, PlayerSelectionArray, COND_None, REPNOTIFY_Always);
}

const TArray<FPlayerSelection>& AChrisGameState::GetPlayerSelection() const
{
	return PlayerSelectionArray;
}

void AChrisGameState::RequestPlayerReadyChange(const APlayerState* RequestingPlayer, bool bReady)
{
	if (!HasAuthority()) return;

	FPlayerSelection* Found = PlayerSelectionArray.FindByPredicate([&](const FPlayerSelection& PlayerSelectionReady)
		{
			return PlayerSelectionReady.IsForPlayer(RequestingPlayer);
		});

	// Player must have a slot before they can ready up
	if (Found)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player tried to ready up!"));
		Found->SetIsReady(bReady);
		ForceNetUpdate();
		OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Player tried to ready but has no slot!"));
	}
}

// Called automatically on clients when the server's PlayerSelectionArray replicates down
void AChrisGameState::OnRep_PlayerSelectionArray()
{
	OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
}
