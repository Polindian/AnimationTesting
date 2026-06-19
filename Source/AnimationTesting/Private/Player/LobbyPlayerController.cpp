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