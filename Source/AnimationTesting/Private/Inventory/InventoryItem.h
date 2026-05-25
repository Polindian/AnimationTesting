// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayEffectTypes.h"
#include "GameplayAbilitySpecHandle.h"
#include "InventoryItem.generated.h"

class UPA_ShopItem;
class UAbilitySystemComponent;



USTRUCT()
struct FInventoryItemHandle
{
	GENERATED_BODY()

public:
	FInventoryItemHandle();
	static FInventoryItemHandle InvalidHandle();
	static FInventoryItemHandle CreateHandle();

	bool IsValid() const;
	uint32 GetHandleId() const { return HandleId; }

private:

	explicit FInventoryItemHandle(uint32 Id);
	UPROPERTY()
	uint32 HandleId;

	static uint32 GenerateNextId();
	static uint32 GetInvalidId();
};

bool operator==(const FInventoryItemHandle& Lhs, const FInventoryItemHandle& Rhs);
uint32 GetTypeHash(const FInventoryItemHandle& Key);


/**
 * 
 */
UCLASS()
class UInventoryItem : public UObject
{
	GENERATED_BODY()
public:
	UInventoryItem();
	void InitItem(const FInventoryItemHandle& NewHandle, const UPA_ShopItem* NewShopItem, UAbilitySystemComponent* AbilitySystemComponent);
	const UPA_ShopItem* GetShopItem() const { return ShopItem; }
	FInventoryItemHandle GetHandle() const { return Handle; }
	bool IsValid() const;

	bool TryActivateGrantedAbility();
	void ApplyConsumeEffect();
	void RemoveGASModifications();

	float GetAbilityCooldownTimeRemaining() const;
	float GetAbilityCooldownDuration() const;
	bool IsOnCooldown() const;
	
	FORCEINLINE int GetStackCount() const { return StackCount; }
	void SetSlot(int NewSlot);
	int GetItemSlot() const { return Slot; }

	bool AddStackCount();
	bool ReduceStackCount();
	bool SetStackCount(int NewStackCount);
	bool IsStackFull() const;
	bool IsForItem(const UPA_ShopItem* Item) const;
	bool IsGrantingAbility(TSubclassOf<class UGameplayAbility> AbilityClass) const;
	bool IsGrantingAnyAbility() const;

	private:
	UPROPERTY()
	const UPA_ShopItem* ShopItem;

	FInventoryItemHandle Handle;

	int StackCount;
	int Slot;

	UAbilitySystemComponent* OwnerAbilitySystemComponent;
	void ApplyGASModifications();

	FActiveGameplayEffectHandle AppliedEquippedEffectHandle;
	FGameplayAbilitySpecHandle GrantedAbilitySpecHandle;
};
