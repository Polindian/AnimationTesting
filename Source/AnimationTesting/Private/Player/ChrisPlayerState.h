// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Player/PlayerInfoTypes.h"
#include "ChrisPlayerState.generated.h"

class UPA_CharacterDefinition;

/**
 * 
 */
UCLASS()
class AChrisPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	AChrisPlayerState();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;


	// Server RPC: client tells the server which character they've picked.
   // The PlayerState then forwards this to ChrisGameState.
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetSelectedCharacterDefinition(const UPA_CharacterDefinition* NewDefinition);

private:
	// Server RPC: client tells the server which character they've picked.
   // The PlayerState then forwards this to ChrisGameState.
	UPROPERTY(Replicated)
	FPlayerSelection PlayerSelection;

	UPROPERTY()
	class AChrisGameState* ChrisGameState;

	void PlayerSelectionUpdated(const TArray<FPlayerSelection>& NewPlayerSelections);
};
