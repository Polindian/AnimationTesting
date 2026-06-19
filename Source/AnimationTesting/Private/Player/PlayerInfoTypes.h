// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "PlayerInfoTypes.generated.h"

class APlayerState;

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
};