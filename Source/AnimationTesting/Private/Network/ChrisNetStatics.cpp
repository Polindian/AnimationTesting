// Christopher Naglik All Rights Reserved


#include "Network/ChrisNetStatics.h"
#include "ChrisNetStatics.h"

FOnlineSessionSettings UChrisNetStatics::GenerateOnlineSessionSettings(const FName& SessionName, const FString SessionSearchId, int Port)
{
	FOnlineSessionSettings OnlineSessionSettings{};
	OnlineSessionSettings.bIsLANMatch = false;
	OnlineSessionSettings.NumPublicConnections = GetPlayerCountPerTeam() * 2;
	OnlineSessionSettings.bShouldAdvertise = true;
	OnlineSessionSettings.bUsesPresence = false;
	OnlineSessionSettings.bAllowJoinViaPresence = false;
	OnlineSessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
	OnlineSessionSettings.bAllowInvites = true;
	OnlineSessionSettings.bAllowJoinInProgress = false;
	OnlineSessionSettings.bUseLobbiesIfAvailable = false;
	OnlineSessionSettings.bUseLobbiesVoiceChatIfAvailable = false;
	OnlineSessionSettings.bUsesStats = true;


	// Attach our launch-time identity as SEARCHABLE metadata. This closes the loop: the backend launched us with -SESSION_SEARCH_ID=X, we advertise X, and a client searching for SESSION_SEARCH_ID == X finds
	// exactly this instance - plus the PORT it needs to connect to.
	OnlineSessionSettings.Set(GetSessionNameKey(), SessionName.ToString(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	OnlineSessionSettings.Set(GetSessionSearchIdKey(), SessionSearchId, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	OnlineSessionSettings.Set(GetPortKey(), Port, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);


	return OnlineSessionSettings;
}

IOnlineSessionPtr UChrisNetStatics::GetSessionPtr()
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		return OnlineSubsystem->GetSessionInterface();
	}

	return nullptr;
}

IOnlineIdentityPtr UChrisNetStatics::GetIdentityPtr()
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		return OnlineSubsystem->GetIdentityInterface();
	}

	return nullptr;
}

uint8 UChrisNetStatics::GetPlayerCountPerTeam()
{
	return 5;
}

bool UChrisNetStatics::IsSessionServer(const UObject* WorldContextObject)
{
	return WorldContextObject->GetWorld()->GetNetMode() == ENetMode::NM_DedicatedServer;
}

FName UChrisNetStatics::GetSessionNameKey()
{
	return FName("SESSION_NAME");
}

FString UChrisNetStatics::GetSessionNameString()
{
	return GetCommandLineArgsAsString(GetSessionNameKey());
}

FString UChrisNetStatics::GetSessionSearchIdString()
{
	return GetCommandLineArgsAsString(GetSessionSearchIdKey());
}

FName UChrisNetStatics::GetSessionSearchIdKey()
{
	return FName("SESSION_SEARCH_ID");
}

int UChrisNetStatics::GetSessionPort()
{
	return GetCommandLineArgsAsInt(GetPortKey());
}

FName UChrisNetStatics::GetPortKey()
{
	return FName("PORT");
}

FString UChrisNetStatics::GetCommandLineArgsAsString(const FName& ParamName)
{
	FString OutValue = "";
	FString CommandLineArg = FString::Printf(TEXT("%s="), *(ParamName.ToString()));
	FParse::Value(FCommandLine::Get(), *CommandLineArg, OutValue);
	return OutValue;
}

int UChrisNetStatics::GetCommandLineArgsAsInt(const FName& ParamName)
{
	int OutValue = 0;
	FString CommandLineArg = FString::Printf(TEXT("%s="), *(ParamName.ToString()));
	FParse::Value(FCommandLine::Get(), *CommandLineArg, OutValue);
	return OutValue;
}
