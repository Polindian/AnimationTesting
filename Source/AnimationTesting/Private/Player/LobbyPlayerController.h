// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Player/MenuPlayerController.h"
#include "LobbyPlayerController.generated.h"


// Delegate fired on the client when the server tells everyone to switch to hero selection
DECLARE_DELEGATE(FOnSwitchToHeroSelection);


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

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestReadyStateChange(bool bNewReady);

	// Client RPC: server tells THIS client to switch to hero selection page
	UFUNCTION(Client, Reliable)
	void Client_StartHeroSelection();

	// Delegate the widget binds to — fires when Client_StartHeroSelection executes
	FOnSwitchToHeroSelection OnSwitchToHeroSelection;

private:
	

};
