// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryItem.h"
#include "InventoryComponent.generated.h"

class UAbilitySystemComponent;
class UPA_ShopItem;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemAddedDelegate, const UInventoryItem* /*NewItem*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemRemovedDelegate, const FInventoryItemHandle& /*ItemHandle*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemStackCountChangeDelegate, const FInventoryItemHandle&, int /*NewCount*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnItemAbilityCommitted, const FInventoryItemHandle&, float /*CooldownDuration*/, float /*TimeRemaining*/);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSkillPurchasedDelegate, const UPA_ShopItem* /*Item*/);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();
	FOnItemAddedDelegate OnItemAdded;
	FOnItemRemovedDelegate OnItemRemoved;
	FOnItemStackCountChangeDelegate OnItemStackCountChanged;
	FOnItemAbilityCommitted OnItemAbilityCommitted;

	FOnSkillPurchasedDelegate OnSkillPurchased;

	void TryActivateItem(const FInventoryItemHandle& ItemHandle);
	bool TryPurchase(const UPA_ShopItem* ItemToPurchase);
	float GetSoul() const;

	FORCEINLINE int GetCapcity() const { return Capacity; }

	void ItemSlotChanged(const FInventoryItemHandle& Handle, int NewSlotNumber);
	UInventoryItem* GetInventoryItemByHandle(const FInventoryItemHandle& Handle) const;

	bool IsFullFor(const UPA_ShopItem* Item) const;
	bool AreAllSlotsOcuppied() const;
	UInventoryItem* GetAvailableStackForItem(const UPA_ShopItem* Item) const;

	void TryActivateItemInSlot(int SlotNumber);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	int Capacity = 5;

	UPROPERTY()
	UAbilitySystemComponent* OwnerAbilitySystemComponent;

	// Check if a non-consumable item was already bought
	bool HasPurchased(const UPA_ShopItem* Item) const { return PurchasedItems.Contains(Item); }

	// Get how many of a consumable the player currently holds
	int32 GetConsumableCount(const UPA_ShopItem* Item) const;

private:
	// Tracks purchased skills/ability upgrades (one-time buys)
	UPROPERTY()
	TArray<const UPA_ShopItem*> PurchasedItems;

	// Tracks consumables in inventory (key = item, value = count)
	UPROPERTY()
	TMap<const UPA_ShopItem*, int32> ConsumableInventory;

	UPROPERTY()
	TMap<FInventoryItemHandle, UInventoryItem*> InventoryMap;

	void AddConsumableToInventory(const UPA_ShopItem* Item);

	void AbilityCommitted(class UGameplayAbility* CommittedAbility);
	
// SERVER 
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Purchase(const UPA_ShopItem* ItemToPurchase);
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ActivateItem(FInventoryItemHandle ItemHandle);
	void GrantItem(const UPA_ShopItem* NewItem);
	void ConsumeItem(UInventoryItem* Item);
	void RemoveItem(UInventoryItem* Item);

// CLIENT
private:
	UFUNCTION(Client,Reliable)
	void Client_ItemAdded(FInventoryItemHandle AssignedHandle, const UPA_ShopItem* Item);

	UFUNCTION(Client, Reliable)
	void Client_ItemRemoved(FInventoryItemHandle ItemHandle);

	UFUNCTION(Client, Reliable)
	void Client_ItemStackCountChanged(FInventoryItemHandle Handle, int NewCount);

	UFUNCTION(Client, Reliable)
	void Client_SkillPurchased(const UPA_ShopItem* Item);
};
