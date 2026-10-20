
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

/**
 * Rules for the pre-match lobby. Exists only on the server.
 * - Uses seamless travel so players and their selections carry over into the arena
 * - Rejects new joins once team selection is over (PreLogin)
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

	virtual void Logout(AController* Exiting) override;

private:
	bool bAcceptingPlayers = true;
};
