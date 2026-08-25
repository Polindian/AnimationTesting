// Christopher Naglik All Rights Reserved


#include "Inventory/InventoryComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAS/CHeroAttributeSet.h"
#include "Framework/ChrisGameMode.h"
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

	if (ItemToPurchase->GetIsConsumable()
		&& GetConsumablePurchaseCount(ItemToPurchase) >= ItemToPurchase->GetMaxStackCount())
	{
		UE_LOG(LogTemp, Warning, TEXT("Match purchase limit reached for: %s"), *ItemToPurchase->GetName());
		return false;
	}

	// Check if player can afford it before sending to server
	if (GetSoul() < ItemToPurchase->GetPrice())
	{
		UE_LOG(LogTemp, Warning, TEXT("Not enough soul for: %s"), *ItemToPurchase->GetName());
		return false;
	}
	// Mark purchased locally so rapid clicks are blocked instantly.
	// Server validates and Client_SkillPurchased confirms later.
	if (!ItemToPurchase->GetIsConsumable())
	{
		PurchasedItems.Add(ItemToPurchase);
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

int32 UInventoryComponent::GetConsumablePurchaseCount(const UPA_ShopItem* Item) const
{
	const int32* Count = ConsumablesPurchasedThisMatch.Find(Item);
	return Count ? *Count : 0;
}

void UInventoryComponent::AddConsumablePurchase(const UPA_ShopItem* Item)
{
	if (!Item) { return; }

	int32& Count = ConsumablesPurchasedThisMatch.FindOrAdd(Item);
	Count = FMath::Min(Count + 1, Item->GetMaxStackCount());

	OnConsumablePurchaseCountChanged.Broadcast(Item, Count);
	Client_ConsumablePurchaseCountChanged(Item, Count);
}

void UInventoryComponent::Client_ConsumablePurchaseCountChanged_Implementation(const UPA_ShopItem* Item, int32 NewCount)
{
	// Listen server already broadcast it directly
	if (GetOwner()->HasAuthority() || !Item) { return; }

	ConsumablesPurchasedThisMatch.Add(Item, NewCount);
	OnConsumablePurchaseCountChanged.Broadcast(Item, NewCount);
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

	// Server-side: block purchases if shop phase has ended
	AChrisGameMode* GM = Cast<AChrisGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM || !GM->IsInShopPhase())
	{
		Client_PurchaseFailed(ItemToPurchase);
		return;
	}

	if (GetSoul() < ItemToPurchase->GetPrice())
	{
		Client_PurchaseFailed(ItemToPurchase);
		return;
	}

	if (IsFullFor(ItemToPurchase))
	{
		Client_PurchaseFailed(ItemToPurchase);
		return;
	}

	// The server had no stack limit at all — only the client did, and its count
	// was always zero, so the cap was never actually enforced anywhere
	if (ItemToPurchase->GetIsConsumable()
		&& GetConsumablePurchaseCount(ItemToPurchase) >= ItemToPurchase->GetMaxStackCount())
	{
		Client_PurchaseFailed(ItemToPurchase);
		return;
	}

	if (ItemToPurchase->GetIsConsumable())
	{
		// Grant first, charge second — the old order cost the player their souls
		// whenever the grant quietly failed
		if (!GrantItem(ItemToPurchase))
		{
			Client_PurchaseFailed(ItemToPurchase);
			return;
		}

		OwnerAbilitySystemComponent->ApplyModToAttribute(
			UCHeroAttributeSet::GetSoulAttribute(), EGameplayModOp::Additive, -ItemToPurchase->GetPrice());

		AddConsumablePurchase(ItemToPurchase);
		return;
	}

	// Skill or Ability Upgrade: applied permanently, never enters the inventory grid
	OwnerAbilitySystemComponent->ApplyModToAttribute(
		UCHeroAttributeSet::GetSoulAttribute(), EGameplayModOp::Additive, -ItemToPurchase->GetPrice());

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
	Client_SkillPurchased(ItemToPurchase);
	OnSkillPurchased.Broadcast(ItemToPurchase);
}

bool UInventoryComponent::Server_Purchase_Validate(const UPA_ShopItem* ItemToPurchase)
{
	return true;
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

bool UInventoryComponent::GrantItem(const UPA_ShopItem* NewItem)
{
	if (!GetOwner()->HasAuthority() || !NewItem)
	{
		return false;
	}

	if (UInventoryItem* StackItem = GetAvailableStackForItem(NewItem))
	{
		// This return value was being ignored: a full stack silently did nothing
		// while the caller had already deducted the price
		if (!StackItem->AddStackCount())
		{
			return false;
		}

		OnItemStackCountChanged.Broadcast(StackItem->GetHandle(), StackItem->GetStackCount());
		Client_ItemStackCountChanged(StackItem->GetHandle(), StackItem->GetStackCount());
		return true;
	}

	// Nothing to stack onto, so it needs a slot of its own
	if (AreAllSlotsOcuppied())
	{
		return false;
	}

	UInventoryItem* InventoryItem = NewObject<UInventoryItem>();
	FInventoryItemHandle NewHandle = FInventoryItemHandle::CreateHandle();
	InventoryItem->InitItem(NewHandle, NewItem, OwnerAbilitySystemComponent);
	InventoryMap.Add(NewHandle, InventoryItem);
	OnItemAdded.Broadcast(InventoryItem);

	Client_ItemAdded(NewHandle, NewItem);
	return true;
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
		OnItemRemoved.Broadcast(Item->GetHandle());
		InventoryMap.Remove(Item->GetHandle());
		Client_ItemRemoved(Item->GetHandle());
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

void UInventoryComponent::Client_PurchaseFailed_Implementation(const UPA_ShopItem* Item)
{
	if (!Item) { return; }

	if (!Item->GetIsConsumable())
	{
		PurchasedItems.Remove(Item);
	}

	OnPurchaseFailed.Broadcast(Item);
}

void UInventoryComponent::Client_SkillPurchased_Implementation(const UPA_ShopItem* Item)
{
	// Server already has it — only update on the remote client.
	if (GetOwner()->HasAuthority())
		return;

	if (!PurchasedItems.Contains(Item))
	{
		PurchasedItems.Add(Item);
	}
	OnSkillPurchased.Broadcast(Item);
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

