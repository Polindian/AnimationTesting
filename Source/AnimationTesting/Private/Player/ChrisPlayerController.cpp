// Christopher Naglik All Rights Reserved


#include "Player/ChrisPlayerController.h"
#include "Player/ChrisPlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/GameplayWidget.h"

void AChrisPlayerController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);
	ChrisPlayerCharacter = Cast<AChrisPlayerCharacter>(NewPawn);
	if (ChrisPlayerCharacter)
	{
		ChrisPlayerCharacter->ServerSideInit();
		ChrisPlayerCharacter->SetGenericTeamId(TeamID);
	}
}

void AChrisPlayerController::AcknowledgePossession(APawn* NewPawn)
{
	Super::AcknowledgePossession(NewPawn);
	ChrisPlayerCharacter = Cast<AChrisPlayerCharacter>(NewPawn);
	if (ChrisPlayerCharacter)
	{
		ChrisPlayerCharacter->ClientSideInit();
		SpawnGameplayWidget();
	}
}

void AChrisPlayerController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID = NewTeamID;
}

FGenericTeamId AChrisPlayerController::GetGenericTeamId() const
{
	return TeamID;
}

void AChrisPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AChrisPlayerController, TeamID);
}

void AChrisPlayerController::SpawnGameplayWidget()
{
	if (!IsLocalPlayerController())
		return;

	GameplayWidget = CreateWidget <UGameplayWidget> (this, GameplayWidgetClass);
	if(GameplayWidget)
	{
		GameplayWidget->AddToViewport();
		GameplayWidget->ConfigureAbilities(ChrisPlayerCharacter->GetAbilities());
	}
}
