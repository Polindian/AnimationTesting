// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ChrisGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class UChrisGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	void StartMatch();
	virtual void Init() override;

/*********************************/
/*          Session Server       */
/*********************************/
public:
	void PlayerJoined(const FUniqueNetIdRepl& UniqueId);
	void PlayerLeft(const FUniqueNetIdRepl& UniqueId);

private:
	void CreateSession();
	void OnSessionCreated(FName SessionName, bool bWasSuccessful);

	FString ServerSessionName;
	int SessionServerPort;

	void TerminateSessionServer();
	void EndSessionCompleted(FName SessionName, bool bWasSuccessful);

	FTimerHandle WaitPlayerJoinTimeoutHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Session")
	float WaitPlayerJoinTimeoutDuration = 300.f;

	void WaitPlayerJoinTimeoutReached();

	TSet<FUniqueNetIdRepl> PlayerRecord;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Map")
	TSoftObjectPtr<UWorld> MainMenuLevel;

	UPROPERTY(EditDefaultsOnly, Category = "Map")
	TSoftObjectPtr<UWorld> LobbyLevel;

	UPROPERTY(EditDefaultsOnly, Category = "Map")
	TSoftObjectPtr<UWorld> Lvl_ThirdPerson;

	void LoadLevelAndListen(TSoftObjectPtr<UWorld>Level);
};
