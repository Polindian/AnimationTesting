// Christopher Naglik All Rights Reserved


#include "Network/ChrisNetStatics.h"
#include "ChrisNetStatics.h"

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
