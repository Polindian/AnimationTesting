// Christopher Naglik All Rights Reserved


#include "Framework/ChrisGameState.h"
#include "Player/LobbyPlayerController.h"
#include "Network/ChrisNetStatics.h"
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


// Assigns a character to the player. Duplicates allowed — multiple players can pick the same hero.
// Blocked if the player is already locked in.
void AChrisGameState::SetCharacterSelected(const APlayerState* SelectingPlayer, const UPA_CharacterDefinition* SelectedDefinition)
{
	if (!HasAuthority()) return;

	FPlayerSelection* FoundPlayerSelection = PlayerSelectionArray.FindByPredicate([&](const FPlayerSelection& PlayerSelected)
		{
			return PlayerSelected.IsForPlayer(SelectingPlayer);
		});

	// Can't change character if already locked in
	if (FoundPlayerSelection && !FoundPlayerSelection->GetIsLockedIn())
	{
		FoundPlayerSelection->SetCharacterDefinition(SelectedDefinition);
		ForceNetUpdate();
		OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
	}
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

void AChrisGameState::SetCharacterDeselected(const APlayerState* RequestingPlayer)
{
	if (!HasAuthority()) return;

	FPlayerSelection* FoundPlayerSelection = PlayerSelectionArray.FindByPredicate([&](const FPlayerSelection& PlayerSelection)
		{
			return PlayerSelection.IsForPlayer(RequestingPlayer);
		});

	if (FoundPlayerSelection && !FoundPlayerSelection->GetIsLockedIn())
	{
		FoundPlayerSelection->SetCharacterDefinition(nullptr);
		ForceNetUpdate();
		OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
	}
}

// Locks or unlocks the player's character selection (START MATCH / RETURN TO SELECTION)
void AChrisGameState::RequestPlayerLockIn(const APlayerState* RequestingPlayer, bool bLockIn)
{
	if (!HasAuthority()) return;

	FPlayerSelection* Found = PlayerSelectionArray.FindByPredicate([&](const FPlayerSelection& PS)
		{
			return PS.IsForPlayer(RequestingPlayer);
		});

	// Player must have a slot and a character selected before locking in
	if (Found && Found->GetCharacterDefinition())
	{
		Found->SetIsLockedIn(bLockIn);
		ForceNetUpdate();
		OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
	}
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

		// Auto-transition: if all players readied and teams balanced, switch everyone
		if (CanStartHeroSelection())
		{
			for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				ALobbyPlayerController* LobbyPC = Cast<ALobbyPlayerController>(It->Get());
				if (LobbyPC)
				{
					LobbyPC->Client_StartHeroSelection();
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Player tried to ready but has no slot!"));
	}
}

bool AChrisGameState::CanStartHeroSelection() const
{
	if (PlayerSelectionArray.Num() == 0) return false;

	int32 PlayersPerTeam = UChrisNetStatics::GetPlayerCountPerTeam();
	int32 RedCount = 0;
	int32 BlueCount = 0;

	for (const FPlayerSelection& PS : PlayerSelectionArray)
	{
		// Every player must be readied
		if (!PS.GetIsReady()) return false;

		// Count team membership
		if (PS.GetPlayerSlot() < PlayersPerTeam)
		{
			RedCount++;
		}
		else
		{
			BlueCount++;
		}
	}

	// Teams must have equal players
	return RedCount == BlueCount;
}


// Called automatically on clients when the server's PlayerSelectionArray replicates down
void AChrisGameState::OnRep_PlayerSelectionArray()
{
	OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
}
