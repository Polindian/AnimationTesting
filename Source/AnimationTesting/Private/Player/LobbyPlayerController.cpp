// Christopher Naglik All Rights Reserved


#include "Player/LobbyPlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Framework/ChrisGameState.h"


// Executes on server: forwards the slot change request to the GameState for authoritative validation
void ALobbyPlayerController::Server_RequestSlotSelectionChange_Implementation(uint8 NewSlotID)
{
	if (!GetWorld()) return;

	AChrisGameState* ChrisGameState = GetWorld()->GetGameState<AChrisGameState>();
	if (!ChrisGameState) return;
	
	// Pass this player's identity and desired slot to the GameState for processing
	ChrisGameState->RequestPlayerSelectionChange(GetPlayerState<APlayerState>(), NewSlotID);
}

// Validation: returning false disconnects the player (anti-cheat). Currently accepts all requests.
bool ALobbyPlayerController::Server_RequestSlotSelectionChange_Validate(uint8 NewSlotID)
{
	return true;
}

void ALobbyPlayerController::Server_RequestReadyStateChange_Implementation(bool bNewReady)
{
	if (!GetWorld()) return;

	AChrisGameState* GameState = GetWorld()->GetGameState<AChrisGameState>();
	if (!GameState) return;
	GameState->RequestPlayerReadyChange(GetPlayerState<APlayerState>(), bNewReady);
}

bool ALobbyPlayerController::Server_RequestReadyStateChange_Validate(bool bNewReady)
{
	return true;
}

// Runs on the owning client — fires the delegate so the widget switches page
void ALobbyPlayerController::Client_StartHeroSelection_Implementation()
{
	OnSwitchToHeroSelection.ExecuteIfBound();
}

// Forwards the character pick to the GameState (duplicates allowed, blocked if locked in)
void ALobbyPlayerController::Server_RequestCharacterSelected_Implementation(const UPA_CharacterDefinition* SelectedDefinition)
{
	if (!GetWorld()) return;

	AChrisGameState* GameState = GetWorld()->GetGameState<AChrisGameState>();
	if (!GameState) return;
	GameState->SetCharacterSelected(GetPlayerState<APlayerState>(), SelectedDefinition);
}

bool ALobbyPlayerController::Server_RequestCharacterSelected_Validate(const UPA_CharacterDefinition* SelectedDefinition)
{
	return true;
}

// Forwards lock-in / unlock request to the GameState
void ALobbyPlayerController::Server_RequestLockIn_Implementation(bool bLockIn)
{
	if (!GetWorld()) return;

	AChrisGameState* GameState = GetWorld()->GetGameState<AChrisGameState>();
	if (!GameState) return;
	GameState->RequestPlayerLockIn(GetPlayerState<APlayerState>(), bLockIn);
}

bool ALobbyPlayerController::Server_RequestLockIn_Validate(bool bLockIn)
{
	return true;
}

ALobbyPlayerController::ALobbyPlayerController()
{
	bAutoManageActiveCameraTarget = false;
}
