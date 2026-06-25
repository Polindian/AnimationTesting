// Christopher Naglik All Rights Reserved


#include "Player/PlayerInfoTypes.h"
#include "GameFramework/PlayerState.h"
#include "Network/ChrisNetStatics.h"

// Sets the members to default empty/invalid values
FPlayerSelection::FPlayerSelection() :Slot{ GetInvalidSlot() }, PlayerUniqueId{ FUniqueNetIdRepl::Invalid() }, PlayerNickname{}, CharacterDefinition{nullptr}
{
}

// Stores the slot, then grab player's unique ID and name from their PlayerState
FPlayerSelection::FPlayerSelection(uint8 InSlot, const APlayerState* InPlayerState):Slot {InSlot}, CharacterDefinition{nullptr}
{
	if (InPlayerState)
	{
		PlayerUniqueId = InPlayerState->GetUniqueId();
		PlayerNickname = InPlayerState->GetPlayerName();
	}
}

// TESTING WITHIN EDITOR PURPOSES

bool FPlayerSelection::IsForPlayer(const APlayerState* PlayerState) const
{
	if (!PlayerState) return false;

#if WITH_EDITOR
	return PlayerState->GetPlayerName() == PlayerNickname;

#else 
	return PlayerState->GetUniqueId() == GetPlayerUniqueId();

#endif

}

bool FPlayerSelection::IsValid() const
{
#if WITH_EDITOR
	return true;

#else
	if (!PlayerUniqueId.IsValid()) return false;

	if (Slot == GetInvalidSlot()) return false;

	if (Slot >= ChrisNetStatics::GetPlayerCountPerTeam()*2) return false;

	return true;

#endif

}

uint8 FPlayerSelection::GetInvalidSlot()
{
	return uint8(255);
}
