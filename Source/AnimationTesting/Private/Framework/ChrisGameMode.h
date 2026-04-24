// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GenericTeamAgentInterface.h"
#include "ChrisGameMode.generated.h"

/**
 * 
 */
UCLASS()
class AChrisGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	virtual APlayerController* SpawnPlayerController(ENetRole InRemoteRole, const FString& Options) override;

private:
	FGenericTeamId GetTeamIDForPlayer(const APlayerController* PlayerController) const;

	AActor* FindNextStartSpotForTeam(const FGenericTeamId& TeamID);

	UPROPERTY(EditDefaultsOnly, Category = "Team")
	TMap<FGenericTeamId, FName> TeamStartSpotTagMap;
};
