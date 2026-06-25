// Christopher Naglik All Rights Reserved


#include "Player/ChrisPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Framework/ChrisGameState.h"
#include "Character/PA_CharacterDefinition.h"
#include "Character/ChrisCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Network/ChrisNetStatics.h"

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

// Called by the engine during seamless/server travel to copy this PlayerState's data into the new PlayerState created on the destination level
void AChrisPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	AChrisPlayerState* NewPlayerState = Cast<AChrisPlayerState>(PlayerState);
	if (NewPlayerState)
	{
		NewPlayerState->PlayerSelection = PlayerSelection;
	}
}

// Returns the character blueprint class the arena GameMode should spawn for this player.
TSubclassOf<APawn> AChrisPlayerState::GetSelectedPawnClass() const
{
	if (PlayerSelection.GetCharacterDefinition())
	{
		return PlayerSelection.GetCharacterDefinition()->LoadCharacterClass();
	}

	return nullptr;
}

// Derives team assignment from slot index
FGenericTeamId AChrisPlayerState::GetTeamIdBasedOnSlot() const
{
	return PlayerSelection.GetPlayerSlot() < UChrisNetStatics::GetPlayerCountPerTeam() ? FGenericTeamId{ 0 } : FGenericTeamId{ 1 };
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
