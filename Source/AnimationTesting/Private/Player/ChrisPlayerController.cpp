// Christopher Naglik All Rights Reserved


#include "Player/ChrisPlayerController.h"
#include "Player/ChrisPlayerCharacter.h"

void AChrisPlayerController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);
	ChrisPlayerCharacter = Cast<AChrisPlayerCharacter>(NewPawn);
	if (ChrisPlayerCharacter)
	{
		ChrisPlayerCharacter->ServerSideInit();
	}
}

void AChrisPlayerController::AcknowledgePossession(APawn* NewPawn)
{
	Super::	AcknowledgePossession(NewPawn);
	ChrisPlayerCharacter = Cast<AChrisPlayerCharacter>(NewPawn);
	if (ChrisPlayerCharacter)
	{
		ChrisPlayerCharacter->ClientSideInit();
	}
}
