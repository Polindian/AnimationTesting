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
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPurchaseFailedDelegate, const UPA_ShopItem* /*Item*/);

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnConsumablePurchaseCountChanged, const UPA_ShopItem*, int32 /*NewCount*/);

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
	FOnPurchaseFailedDelegate OnPurchaseFailed;

	FOnConsumablePurchaseCountChanged OnConsumablePurchaseCountChanged;

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

	// How many of this consumable the player has bought this match — a cap, not a live count, so drinking one does not free the purchase back up
	int32 GetConsumablePurchaseCount(const UPA_ShopItem* Item) const;

	UFUNCTION(Client, Reliable)
	void Client_ConsumablePurchaseCountChanged(const UPA_ShopItem* Item, int32 NewCount);

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


private:
	// Tracks purchased skills/ability upgrades (one-time buys)
	UPROPERTY()
	TArray<const UPA_ShopItem*> PurchasedItems;

	// Purchases per consumable this match (key = item, value = total bought)
	UPROPERTY()
	TMap<const UPA_ShopItem*, int32> ConsumablesPurchasedThisMatch;


	void AddConsumablePurchase(const UPA_ShopItem* Item);


	
	UPROPERTY()
	TMap<FInventoryItemHandle, UInventoryItem*> InventoryMap;

	void AbilityCommitted(class UGameplayAbility* CommittedAbility);
	
// SERVER 
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Purchase(const UPA_ShopItem* ItemToPurchase);
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ActivateItem(FInventoryItemHandle ItemHandle);

	// Returns false if the item couldn't actually be placed — caller must not charge
	bool GrantItem(const UPA_ShopItem* NewItem);

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

	UFUNCTION(Client, Reliable)
	void Client_PurchaseFailed(const UPA_ShopItem* Item);
};
