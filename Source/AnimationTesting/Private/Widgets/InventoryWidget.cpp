// Christopher Naglik All Rights Reserved

#include "Widgets/InventoryWidget.h"
#include "Widgets/InventoryItemWidget.h"
#include "Inventory/InventoryComponent.h"

void UInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Build the array from the 5 bound slots
    ItemWidgets = { Slot_0, Slot_1, Slot_2, Slot_3, Slot_4 };

    // Assign slot numbers
    for (int i = 0; i < ItemWidgets.Num(); ++i)
    {
        if (ItemWidgets[i])
        {
            ItemWidgets[i]->SetSlotNumber(i);
        }
    }

    if (APawn* OwnerPawn = GetOwningPlayerPawn())
    {
        InventoryComponent = OwnerPawn->GetComponentByClass<UInventoryComponent>();
        if (InventoryComponent)
        {
            InventoryComponent->OnItemAdded.AddUObject(this, &UInventoryWidget::ItemAdded);
			InventoryComponent->OnItemRemoved.AddUObject(this, &UInventoryWidget::ItemRemoved);
            InventoryComponent->OnItemStackCountChanged.AddUObject(this, &UInventoryWidget::ItemStackCountChanged);
            InventoryComponent->OnItemAbilityCommitted.AddUObject(this, &UInventoryWidget::ItemAbilityCommitted);
        }
    }
}

void UInventoryWidget::ItemAdded(const UInventoryItem* InventoryItem)
{
    if (!InventoryItem)
        return;

    if (UInventoryItemWidget* NextAvailableSlot = GetNextAvailableSlot())
    {
        NextAvailableSlot->UpdateInventoryItem(InventoryItem);
        PopulatedItemEntryWidgets.Add(InventoryItem->GetHandle(), NextAvailableSlot);
        if (InventoryComponent)
        {
            InventoryComponent->ItemSlotChanged(InventoryItem->GetHandle(), NextAvailableSlot->GetSlotNumber());
        }
    }
}

void UInventoryWidget::ItemRemoved(const FInventoryItemHandle& Handle)
{
    UInventoryItemWidget** FoundWidget = PopulatedItemEntryWidgets.Find(Handle);
    if (FoundWidget && *FoundWidget)
    {
        (*FoundWidget)->EmptySlot();
        PopulatedItemEntryWidgets.Remove(Handle);
    }
}

void UInventoryWidget::ItemAbilityCommitted(const FInventoryItemHandle& ItemHandle, float CooldownDuration, float CooldownTimeRemaining)
{
    UE_LOG(LogTemp, Warning, TEXT("ItemAbilityCommitted: Handle=%u, Duration=%.1f, Remaining=%.1f"),
        ItemHandle.GetHandleId(), CooldownDuration, CooldownTimeRemaining);
    UInventoryItemWidget** FoundWidget = PopulatedItemEntryWidgets.Find(ItemHandle);
    if (FoundWidget && *FoundWidget)
    {
        (*FoundWidget)->StartCooldown(CooldownDuration, CooldownTimeRemaining);
    }
}

void UInventoryWidget::ItemStackCountChanged(const FInventoryItemHandle& Handle, int NewCount)
{
	UInventoryItemWidget** FoundWidget = PopulatedItemEntryWidgets.Find(Handle);
    if(FoundWidget)
    {
        (*FoundWidget)->UpdateStackCount();
	}
}

UInventoryItemWidget* UInventoryWidget::GetNextAvailableSlot() const
{
    for (UInventoryItemWidget* Widget : ItemWidgets)
    {
        if (Widget && Widget->IsEmpty())
        {
            return Widget;
        }
    }

    return nullptr;
}