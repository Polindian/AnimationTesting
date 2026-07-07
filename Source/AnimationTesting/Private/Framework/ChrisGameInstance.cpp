// Christopher Naglik All Rights Reserved


#include "Framework/ChrisGameInstance.h"
#include "ChrisGameInstance.h"
#include "Network/ChrisNetStatics.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"

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
	IOnlineSessionPtr SessionPtr = UChrisNetStatics::GetSessionPtr();
	if (SessionPtr)
	{
		ServerSessionName = UChrisNetStatics::GetSessionNameString();
		FString SessionSearchId = UChrisNetStatics::GetSessionSearchIdString();
		SessionServerPort = UChrisNetStatics::GetSessionPort();

		FOnlineSessionSettings OnlineSessionSetting = UChrisNetStatics::GenerateOnlineSessionSettings(FName(ServerSessionName), *SessionSearchId, SessionServerPort);
		if (!SessionPtr->CreateSession(0, FName(ServerSessionName), OnlineSessionSetting))
		{
			UE_LOG(LogTemp, Warning, TEXT("Session Creation FAILED right away!"));
		}

		UE_LOG(LogTemp, Warning, TEXT("Creating Session with Name: %s, SearchId: %s, Port: %d"), *ServerSessionName, *SessionSearchId, SessionServerPort);
	}
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
