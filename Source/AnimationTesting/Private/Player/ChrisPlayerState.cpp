// Christopher Naglik All Rights Reserved


#include "Player/ChrisPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Framework/ChrisGameState.h"
#include "Kismet/GameplayStatics.h"

AChrisPlayerState::AChrisPlayerState()
{
	bReplicates = true;
	SetNetUpdateFrequency(100.f);
}

void AChrisPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AChrisPlayerState, PlayerSelection);
}

void AChrisPlayerState::BeginPlay()
{
	Super::BeginPlay();

	// Cache the GameState and subscribe to lobby data changes
	ChrisGameState = Cast<AChrisGameState>(UGameplayStatics::GetGameState(this));

	if (ChrisGameState)
	{
		// When any player's selection changes, we check if ours changed too
		ChrisGameState->OnPlayerSelectionUpdated.AddUObject(this, &AChrisPlayerState::PlayerSelectionUpdated);
	}
}


// Server RPC: called when the player picks a new character on the hero selection page.
// First deselects the old character (if any), then selects the new one.
void AChrisPlayerState::Server_SetSelectedCharacterDefinition_Implementation(const UPA_CharacterDefinition* NewDefinition)
{
	if (!ChrisGameState)
		return;

	if (!NewDefinition)
		return;

	if (PlayerSelection.GetCharacterDefinition())
	{
		ChrisGameState->SetCharacterDeselected(this);
	}

	PlayerSelection.SetCharacterDefinition(NewDefinition);
	ChrisGameState->SetCharacterSelected(this, NewDefinition);
}

bool AChrisPlayerState::Server_SetSelectedCharacterDefinition_Validate(const UPA_CharacterDefinition* NewDefinition)
{
	return true;
}

void AChrisPlayerState::PlayerSelectionUpdated(const TArray<FPlayerSelection>& NewPlayerSelections)
{
	for (const FPlayerSelection& NewPlayerSelection : NewPlayerSelections)
	{
		if (NewPlayerSelection.IsForPlayer(this))
		{
			PlayerSelection = NewPlayerSelection;
		}
	}
}
