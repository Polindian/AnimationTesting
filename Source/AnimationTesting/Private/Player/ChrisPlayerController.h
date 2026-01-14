// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ChrisPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class AChrisPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	// only called on the server
	void OnPossess(APawn* NewPawn) override;
	// only called on the client and listening server
	void AcknowledgePossession(APawn* NewPawn) override;
private:
	UPROPERTY()
	class AChrisPlayerCharacter* ChrisPlayerCharacter;
};
