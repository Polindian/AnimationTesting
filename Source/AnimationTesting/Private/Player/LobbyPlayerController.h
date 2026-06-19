// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Player/MenuPlayerController.h"
#include "LobbyPlayerController.generated.h"

/**
 * Player controller for the lobby level.
 * Provides a Server RPC so clients can request team slot changes.
 */
UCLASS()
class ALobbyPlayerController : public AMenuPlayerController
{
	GENERATED_BODY()
public:

	// Server RPC: client requests to move to a new team slot.
	// Validated and executed on the server only.
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestSlotSelectionChange(uint8 NewSlotID);

private:
	

};
