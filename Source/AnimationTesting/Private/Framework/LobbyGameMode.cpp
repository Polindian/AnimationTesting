// Christopher Naglik All Rights Reserved


#include "Framework/LobbyGameMode.h"
#include "Network/ChrisGameSession.h"

ALobbyGameMode::ALobbyGameMode()
{
	bUseSeamlessTravel = true;
	GameSessionClass = AChrisGameSession::StaticClass();
}


void ALobbyGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	// Search results are a few seconds stale, so a client can still try to join
	// after we've unlisted. A non-empty ErrorMessage refuses the connection
	if (!bAcceptingPlayers)
	{
		ErrorMessage = TEXT("This match has already started.");
		UE_LOG(LogTemp, Warning, TEXT("[Lobby] Rejected a join — team selection is over"));
	}
}