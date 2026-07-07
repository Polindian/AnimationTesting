// Christopher Naglik All Rights Reserved


#include "Framework/ChrisGameInstance.h"
#include "ChrisGameInstance.h"
#include "Network/ChrisNetStatics.h"

// Only the server (dedicated or listen) should initiate a map travel
void UChrisGameInstance::StartMatch()
{
	if (GetWorld()->GetNetMode() == ENetMode::NM_DedicatedServer || GetWorld()->GetNetMode() == ENetMode::NM_ListenServer)
	{
		LoadLevelAndListen(Lvl_ThirdPerson);
	}
}

void UChrisGameInstance::Init()
{
	Super::Init();
	if (GetWorld()->IsEditorWorld())
		return;

	if (UChrisNetStatics::IsSessionServer(this))
	{
		CreateSession();
	}
}

void UChrisGameInstance::CreateSession()
{
	ServerSessionName = UChrisNetStatics::GetSessionNameString();
	FString SessionSearchId = UChrisNetStatics::GetSessionSearchIdString();
	SessionServerPort = UChrisNetStatics::GetSessionPort();

	UE_LOG(LogTemp, Warning, TEXT("Creating Session with Name: %s, SearchId: %s, Port: %d"), *ServerSessionName, *SessionSearchId, SessionServerPort);
}

void UChrisGameInstance::LoadLevelAndListen(TSoftObjectPtr<UWorld> Level)
{
	// Convert the soft object path into a mappable package name 
	const FName LevelURL = FName(*FPackageName::ObjectPathToPackageName(Level.ToString()));

	// ServerTravel with "?listen" so the server accepts client connections on the new map
	if (LevelURL != "")
	{
		GetWorld()->ServerTravel(LevelURL.ToString() + "?listen");
	}
}
