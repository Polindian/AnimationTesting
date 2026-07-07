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
private:
	void CreateSession();
	FString ServerSessionName;
	int SessionServerPort;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Map")
	TSoftObjectPtr<UWorld> MainMenuLevel;

	UPROPERTY(EditDefaultsOnly, Category = "Map")
	TSoftObjectPtr<UWorld> LobbyLevel;

	UPROPERTY(EditDefaultsOnly, Category = "Map")
	TSoftObjectPtr<UWorld> Lvl_ThirdPerson;

	void LoadLevelAndListen(TSoftObjectPtr<UWorld>Level);
};
