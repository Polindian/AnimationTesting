// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "ChrisNetStatics.generated.h"

/**
 * 
 */
UCLASS()
class UChrisNetStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	static FOnlineSessionSettings GenerateOnlineSessionSettings(const FName& SessionName, const FString SessionSearchId, int Port);

	static IOnlineSessionPtr GetSessionPtr();
	static IOnlineIdentityPtr GetIdentityPtr();

	static uint8 GetPlayerCountPerTeam();
	static bool IsSessionServer(const UObject* WorldContextObject);

	static FName GetSessionNameKey();
	static FString GetSessionNameString();

	static FString GetSessionSearchIdString();
	static FName GetSessionSearchIdKey();

	static int GetSessionPort();
	static FName GetPortKey();

	static FString GetCommandLineArgsAsString(const FName& ParamName);
	static int GetCommandLineArgsAsInt(const FName& ParamName);
};
