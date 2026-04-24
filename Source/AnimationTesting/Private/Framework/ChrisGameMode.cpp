// Christopher Naglik All Rights Reserved


#include "Framework/ChrisGameMode.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"


APlayerController* AChrisGameMode::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
	APlayerController* NewPlayerController = Super::SpawnPlayerController(InRemoteRole, Options);
	IGenericTeamAgentInterface* NewPlayerTeamInterface = Cast<IGenericTeamAgentInterface>(NewPlayerController);
	FGenericTeamId TeamId = GetTeamIDForPlayer(NewPlayerController);
	if (NewPlayerTeamInterface)
	{
		NewPlayerTeamInterface->SetGenericTeamId(TeamId);
	}
	
	NewPlayerController->StartSpot = FindNextStartSpotForTeam(TeamId);
	
	return NewPlayerController;
}

FGenericTeamId AChrisGameMode::GetTeamIDForPlayer(const APlayerController* PlayerController) const
{
	static int PlayerCount = 0;
	++PlayerCount;
	return FGenericTeamId(PlayerCount % 2);
}

AActor* AChrisGameMode::FindNextStartSpotForTeam(const FGenericTeamId& TeamID)
{
	const FName* StartSpotTag = TeamStartSpotTagMap.Find(TeamID);
	if (!StartSpotTag)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();

	for(TActorIterator<APlayerStart> It(World); It; ++It)
	{
		if (It->PlayerStartTag == *StartSpotTag)
		{
			It->PlayerStartTag = FName("Taken");
			return *It;
		}
	}
	return nullptr;
}


