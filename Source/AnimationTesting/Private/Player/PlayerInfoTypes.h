// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "PlayerInfoTypes.generated.h"

class APlayerState;
class UPA_CharacterDefinition;

USTRUCT()
struct FPlayerSelection
{
	GENERATED_BODY()

public:
	FPlayerSelection(); //  For invalid slots
	FPlayerSelection(uint8 InSlot, const APlayerState* InPlayerState); // For valid slots

	FORCEINLINE void SetSlot(uint8 NewSlot) { Slot = NewSlot; }
	FORCEINLINE uint8 GetPlayerSlot() const { return Slot; }

	FORCEINLINE FUniqueNetIdRepl GetPlayerUniqueId() const { return PlayerUniqueId; }
	FORCEINLINE FString GetPlayerNickname() const { return PlayerNickname; }

	FORCEINLINE bool GetIsReady() const { return bIsReady; }
	FORCEINLINE void SetIsReady(bool bReady) { bIsReady = bReady; }

	FORCEINLINE const UPA_CharacterDefinition* GetCharacterDefinition() const { return CharacterDefinition; }
	FORCEINLINE void SetCharacterDefinition(const UPA_CharacterDefinition* NewCharacterDefinition) { CharacterDefinition = NewCharacterDefinition; }

	// Lock-in state — once locked in, character selection cannot be changed
	FORCEINLINE bool GetIsLockedIn() const { return bIsLockedIn; }
	FORCEINLINE void SetIsLockedIn(bool bLocked) { bIsLockedIn = bLocked; }

	bool IsForPlayer(const APlayerState* PlayerState) const;
	bool IsValid() const;

	static uint8 GetInvalidSlot();

private:
	UPROPERTY()
	uint8 Slot;

	UPROPERTY()
	FUniqueNetIdRepl PlayerUniqueId;

	UPROPERTY()
	FString PlayerNickname;

	UPROPERTY()
	bool bIsReady = false;

	UPROPERTY()
	const UPA_CharacterDefinition* CharacterDefinition;

	UPROPERTY()
	bool bIsLockedIn = false;
};