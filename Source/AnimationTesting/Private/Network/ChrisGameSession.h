// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameSession.h"
#include "ChrisGameSession.generated.h"

/**
 * 
 */
UCLASS()
class AChrisGameSession : public AGameSession
{
	GENERATED_BODY()
	
public:
	virtual bool ProcessAutoLogin() override;
	
	virtual void RegisterPlayer(APlayerController* NewPlayer, const FUniqueNetIdRepl& UniqueId, bool bWasFromInvite) override;
	virtual void UnregisterPlayer(FName FromSessionName, const FUniqueNetIdRepl& UniqueId) override;
};
