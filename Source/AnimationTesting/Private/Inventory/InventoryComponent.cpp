// Christopher Naglik All Rights Reserved


#include "Inventory/InventoryComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAS/CHeroAttributeSet.h"
#include "Inventory/PA_ShopItem.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UInventoryComponent::TryActivateItem(const FInventoryItemHandle& ItemHandle)
{
	UInventoryItem* InventoryItem = GetInventoryItemByHandle(ItemHandle);
	if(!InventoryItem)
	{
		return;
	}

	Server_ActivateItem(ItemHandle);
}

bool UInventoryComponent::TryPurchase(const UPA_ShopItem* ItemToPurchase)
{
	if (!OwnerAbilitySystemComponent || !ItemToPurchase)
		return false;

	if (!ItemToPurchase->GetIsConsumable() && HasPurchased(ItemToPurchase))
	{
		UE_LOG(LogTemp, Warning, TEXT("Already purchased: %s"), *ItemToPurchase->GetName());
		return false;
	}

	if (ItemToPurchase->GetIsConsumable() && GetConsumableCount(ItemToPurchase) >= ItemToPurchase->GetMaxStackCount())
	{
		UE_LOG(LogTemp, Warning, TEXT("Max stack reached for: %s"), *ItemToPurchase->GetName());
		return false;
	}

	// Check if player can afford it before sending to server
	if (GetSoul() < ItemToPurchase->GetPrice())
	{
		UE_LOG(LogTemp, Warning, TEXT("Not enough soul for: %s"), *ItemToPurchase->GetName());
		return false;
	}

	Server_Purchase(ItemToPurchase);
	return true;
}

float UInventoryComponent::GetSoul() const
{
	bool bFound = false;
	if (OwnerAbilitySystemComponent)
	{
		float Soul = OwnerAbilitySystemComponent->GetGameplayAttributeValue(UCHeroAttributeSet::GetSoulAttribute(), bFound);
		if (bFound)
		{
			return Soul;
		}
	}

	return 0.f;
}

void UInventoryComponent::ItemSlotChanged(const FInventoryItemHandle& Handle, int NewSlotNumber)
{
	if(UInventoryItem* FoundItem = GetInventoryItemByHandle(Handle))
	{
		FoundItem->SetSlot(NewSlotNumber);
	}
}

UInventoryItem* UInventoryComponent::GetInventoryItemByHandle(const FInventoryItemHandle& Handle) const
{
	UInventoryItem* const* FoundItem = InventoryMap.Find(Handle);
	if(FoundItem)
	{
		return *FoundItem;
	}
	return nullptr;
}

bool UInventoryComponent::IsFullFor(const UPA_ShopItem* Item) const
{
	if (!Item)
		return true;  // was "return;" — must return a bool
	if (AreAllSlotsOcuppied())
	{
		return GetAvailableStackForItem(Item) == nullptr;
	}
	return false;
}

bool UInventoryComponent::AreAllSlotsOcuppied() const
{
	return (InventoryMap.Num() >= GetCapcity());
	
}

UInventoryItem* UInventoryComponent::GetAvailableStackForItem(const UPA_ShopItem* Item) const
{
	if(!Item->GetIsStackable())
		return nullptr;

	for(const TPair<FInventoryItemHandle, UInventoryItem*>& ItemPair : InventoryMap)
	{
		if(ItemPair.Value && ItemPair.Value->IsForItem(Item) && !ItemPair.Value->IsStackFull())
		{
			return ItemPair.Value;
		}
	}

	return nullptr;
}

void UInventoryComponent::TryActivateItemInSlot(int SlotNumber)
{
	for(TPair<FInventoryItemHandle, UInventoryItem*>& ItemPair : InventoryMap)
	{
		if(ItemPair.Value->GetItemSlot() == SlotNumber)
		{
			Server_ActivateItem(ItemPair.Key);
			return;
		}
	}
}

// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	OwnerAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (OwnerAbilitySystemComponent)
	{
		OwnerAbilitySystemComponent->AbilityCommittedCallbacks.AddUObject(this, &UInventoryComponent::AbilityCommitted);
	}
	
}

void UInventoryComponent::Server_Purchase_Implementation(const UPA_ShopItem* ItemToPurchase)
{
	if (!ItemToPurchase)
	{
		return;
	}

	if (GetSoul() < ItemToPurchase->GetPrice())
	{
		return;
	}

	if(IsFullFor(ItemToPurchase))
	{
		return;
	}

	OwnerAbilitySystemComponent->ApplyModToAttribute(UCHeroAttributeSet::GetSoulAttribute(), EGameplayModOp::Additive, -ItemToPurchase->GetPrice());
	UE_LOG(LogTemp, Warning, TEXT("Bought Item: %s"), *(ItemToPurchase->GetName()));

	if (ItemToPurchase->GetIsConsumable())
	{
		// Consumable: add to inventory widget and track stack count
		GrantItem(ItemToPurchase);
		AddConsumableToInventory(ItemToPurchase);
	}
	else
	{
		// Skill or Ability Upgrade: apply effect permanently, mark as purchased, not added to inventory widget
		if (ItemToPurchase->GetEquippedEffect())
		{
			FGameplayEffectContextHandle Context = OwnerAbilitySystemComponent->MakeEffectContext();
			FGameplayEffectSpecHandle Spec = OwnerAbilitySystemComponent->MakeOutgoingSpec(ItemToPurchase->GetEquippedEffect(), 1, Context);
			OwnerAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
		if (ItemToPurchase->GetGrantedAbility())
		{
			OwnerAbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(ItemToPurchase->GetGrantedAbility(), 1));
		}
		PurchasedItems.Add(ItemToPurchase);
	}
}

bool UInventoryComponent::Server_Purchase_Validate(const UPA_ShopItem* ItemToPurchase)
{
	return true;
}

void UInventoryComponent::AddConsumableToInventory(const UPA_ShopItem* Item)
{
	int32& Count = ConsumableInventory.FindOrAdd(Item);
	Count = FMath::Min(Count + 1, Item->GetMaxStackCount());
	UE_LOG(LogTemp, Warning, TEXT("Added consumable: %s (now %d)"), *Item->GetName(), Count);
}


void UInventoryComponent::AbilityCommitted(class UGameplayAbility* CommittedAbility)
{
	if (!CommittedAbility)
		return;

	UE_LOG(LogTemp, Warning, TEXT("AbilityCommitted called for: %s"), *CommittedAbility->GetClass()->GetName());

	float CooldownTimeRemaining = 0.f;
	float CooldownDuration = 0.f;

	CommittedAbility->GetCooldownTimeRemainingAndDuration(CommittedAbility->GetCurrentAbilitySpecHandle(), CommittedAbility->GetCurrentActorInfo(), CooldownTimeRemaining, CooldownDuration);

	for (TPair<FInventoryItemHandle, UInventoryItem*>& ItemPair : InventoryMap)
	{
		if (!ItemPair.Value)
			return;

		if (ItemPair.Value->IsGrantingAbility(CommittedAbility->GetClass()))
		{
			OnItemAbilityCommitted.Broadcast(ItemPair.Key, CooldownDuration, CooldownTimeRemaining);
		}
	}
}

void UInventoryComponent::Server_ActivateItem_Implementation(FInventoryItemHandle ItemHandle)
{
	UInventoryItem* InventoryItem = GetInventoryItemByHandle(ItemHandle);
	if (!InventoryItem)
		return;

	if (InventoryItem->IsOnCooldown())
		return;

	InventoryItem->TryActivateGrantedAbility();
	const UPA_ShopItem* Item = InventoryItem->GetShopItem();
	if (Item->GetIsConsumable())
	{
		ConsumeItem(InventoryItem);
	}
}

bool UInventoryComponent::Server_ActivateItem_Validate(FInventoryItemHandle ItemHandle)
{
	return true;
}

void UInventoryComponent::GrantItem(const UPA_ShopItem* NewItem)
{
	if(!GetOwner()->HasAuthority())
	{
		return;
	}

	if(UInventoryItem* StackItem = GetAvailableStackForItem(NewItem))
	{
		StackItem->AddStackCount();
		OnItemStackCountChanged.Broadcast(StackItem->GetHandle(), StackItem->GetStackCount());
		Client_ItemStackCountChanged(StackItem->GetHandle(), StackItem->GetStackCount());
	}
	else
	{
		UInventoryItem* InventoryItem = NewObject <UInventoryItem>();
		FInventoryItemHandle NewHandle = FInventoryItemHandle::CreateHandle();
		InventoryItem->InitItem(NewHandle, NewItem, OwnerAbilitySystemComponent);
		InventoryMap.Add(NewHandle, InventoryItem);
		OnItemAdded.Broadcast(InventoryItem);
		UE_LOG(LogTemp, Warning, TEXT("Server Added Item: %s, with Id: %d"), *(InventoryItem->GetShopItem()->GetName()), NewHandle.GetHandleId());

		Client_ItemAdded(NewHandle, NewItem);
	}

	
}

void UInventoryComponent::ConsumeItem(UInventoryItem* Item)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (!Item)
		return;

	Item->ApplyConsumeEffect();
	if(!Item->ReduceStackCount())
	{
		RemoveItem(Item);
	}
	else
	{
		OnItemStackCountChanged.Broadcast(Item->GetHandle(), Item->GetStackCount());
		Client_ItemStackCountChanged(Item->GetHandle(), Item->GetStackCount());
	}
}

void UInventoryComponent::RemoveItem(UInventoryItem* Item)
{
	if(!GetOwner()->HasAuthority())
	{
		return;
	}

	Item->RemoveGASModifications();
	OnItemRemoved.Broadcast(Item->GetHandle());
	InventoryMap.Remove(Item->GetHandle());
	Client_ItemRemoved(Item->GetHandle());
}

void UInventoryComponent::Client_ItemRemoved_Implementation(FInventoryItemHandle ItemHandle)
{
	if(GetOwner()->HasAuthority())
	{
		return;
	}

	UInventoryItem* InventoryItem = GetInventoryItemByHandle(ItemHandle);
	if(!InventoryItem)
	{
		return;
	}

	OnItemRemoved.Broadcast(ItemHandle);
	InventoryMap.Remove(ItemHandle);
}

void UInventoryComponent::Client_ItemStackCountChanged_Implementation(FInventoryItemHandle Handle, int NewCount)
{
	if (GetOwner()->HasAuthority())
	{
		return;
	}

	UInventoryItem* FoundItem = GetInventoryItemByHandle(Handle);
	if(FoundItem)
	{
		FoundItem->SetStackCount(NewCount);
		OnItemStackCountChanged.Broadcast(Handle, NewCount);
	}
}

void UInventoryComponent::Client_ItemAdded_Implementation(FInventoryItemHandle AssignedHandle, const UPA_ShopItem* Item)
{
	if (GetOwner()->HasAuthority())
	{
		return;
	}

	UInventoryItem* InventoryItem = NewObject <UInventoryItem>();
	InventoryItem->InitItem(AssignedHandle, Item, OwnerAbilitySystemComponent);
	InventoryMap.Add(AssignedHandle, InventoryItem);
	OnItemAdded.Broadcast(InventoryItem);
	UE_LOG(LogTemp, Warning, TEXT("Client Added Item: %s, with Id: %d"), *(InventoryItem->GetShopItem()->GetName()), AssignedHandle.GetHandleId());
}

int32 UInventoryComponent::GetConsumableCount(const UPA_ShopItem* Item) const
{
	const int32* Count = ConsumableInventory.Find(Item);
	return Count ? *Count : 0;
}

