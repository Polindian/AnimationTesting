
// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/InventoryItem.h"
#include "InventoryWidget.generated.h"

class UInventoryItemWidget;

UCLASS()
class UInventoryWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeConstruct() override;


private:
    // 5 manually placed inventory slots bound by name
    UPROPERTY(meta = (BindWidget))
    UInventoryItemWidget* Slot_0;
    UPROPERTY(meta = (BindWidget))
    UInventoryItemWidget* Slot_1;
    UPROPERTY(meta = (BindWidget))
    UInventoryItemWidget* Slot_2;
    UPROPERTY(meta = (BindWidget))
    UInventoryItemWidget* Slot_3;
    UPROPERTY(meta = (BindWidget))
    UInventoryItemWidget* Slot_4;

    UPROPERTY()
    class UInventoryComponent* InventoryComponent;

    // Array built from the 5 bound slots for easy iteration
    TArray<UInventoryItemWidget*> ItemWidgets;
    TMap<FInventoryItemHandle, UInventoryItemWidget*> PopulatedItemEntryWidgets;

    void ItemAdded(const UInventoryItem* InventoryItem);
	void ItemRemoved(const FInventoryItemHandle& Handle);
    void ItemAbilityCommitted(const FInventoryItemHandle& ItemHandle, float CooldownDuration, float CooldownTimeRemaining);
    void ItemStackCountChanged(const FInventoryItemHandle& Handle, int NewCount);
    UInventoryItemWidget* GetNextAvailableSlot() const;
};
