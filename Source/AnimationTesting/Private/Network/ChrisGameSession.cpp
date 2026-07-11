// Christopher Naglik All Rights Reserved


#include "Network/ChrisGameSession.h"
#include "ChrisGameSession.h"
#include "Framework/ChrisGameInstance.h"

bool AChrisGameSession::ProcessAutoLogin()
{
	return true;
}

void AChrisGameSession::RegisterPlayer(APlayerController* NewPlayer, const FUniqueNetIdRepl& UniqueId, bool bWasFromInvite)
{
	Super::RegisterPlayer(NewPlayer, UniqueId, bWasFromInvite);
	if(UChrisGameInstance* GameInstance = GetGameInstance<UChrisGameInstance>())
	{
		GameInstance->PlayerJoined(UniqueId);
	}
}

void AChrisGameSession::UnregisterPlayer(FName FromSessionName, const FUniqueNetIdRepl& UniqueId)
{
	Super::UnregisterPlayer(FromSessionName, UniqueId);
	if (UChrisGameInstance* GameInstance = GetGameInstance<UChrisGameInstance>())
	{
		GameInstance->PlayerLeft(UniqueId);
	}
}



