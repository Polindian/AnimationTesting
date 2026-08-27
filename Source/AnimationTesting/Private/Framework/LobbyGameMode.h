// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

/**
 * Game mode for the pre-match lobby level.
 * Enables seamless travel for server-to-game transitions.
 */
UCLASS()
class ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	ALobbyGameMode();

	virtual void PreLogin(const FString& Options, const FString& Address,
		const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	// Flipped once the lobby leaves team selection
	void SetAcceptingPlayers(bool bAccepting) { bAcceptingPlayers = bAccepting; }

private:
	bool bAcceptingPlayers = true;
};
