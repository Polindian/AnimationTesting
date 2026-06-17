// Christopher Naglik All Rights Reserved


#include "Widgets/InventoryItemWidget.h"
#include "Inventory/InventoryItem.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Inventory/PA_ShopItem.h"
#include "Widgets/ItemToolTip.h"

void UInventoryItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
	EmptySlot();
}

bool UInventoryItemWidget::IsEmpty() const
{
	return !InventoryItem || !(InventoryItem->IsValid());
}

void UInventoryItemWidget::SetSlotNumber(int NewSlotNumber)
{
	SlotNumber = NewSlotNumber;
}

void UInventoryItemWidget::UpdateInventoryItem(const UInventoryItem* Item)
{
	InventoryItem = Item;
	if(!InventoryItem || !(InventoryItem->IsValid() || InventoryItem->GetStackCount() <= 0))
	{
		EmptySlot();
		return;
	}

	SetIcon(Item->GetShopItem()->GetIcon());
	if (InventoryItem->GetShopItem()->GetIsStackable())
	{
		StackCountText->SetVisibility(ESlateVisibility::Visible);
		UpdateStackCount();
	}
	else
	{
		StackCountText->SetVisibility(ESlateVisibility::Hidden);
	}

	ClearCooldown();

	if (InventoryItem->IsGrantingAnyAbility())
	{
		float AbilityCooldownRemaining = InventoryItem->GetAbilityCooldownTimeRemaining();
		float AbilityCooldownDuration = InventoryItem->GetAbilityCooldownDuration();

		if (AbilityCooldownRemaining > 0.f)
		{
			StartCooldown(AbilityCooldownDuration, AbilityCooldownRemaining);
		}

		CooldownDurationText->SetVisibility(AbilityCooldownDuration == 0.f ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
		CooldownDurationText->SetText(FText::AsNumber(AbilityCooldownDuration));
	}

	else 
	{
		CooldownDurationText->SetVisibility(ESlateVisibility::Hidden);
		CooldownCountText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UInventoryItemWidget::EmptySlot()
{
	ClearCooldown();
	InventoryItem = nullptr;
	SetIcon(EmptyTexture);
	SetToolTip(nullptr);

	StackCountText->SetVisibility(ESlateVisibility::Hidden);
	CooldownCountText->SetVisibility(ESlateVisibility::Hidden);
	CooldownDurationText->SetVisibility(ESlateVisibility::Hidden);
}

void UInventoryItemWidget::UpdateStackCount()
{
	if (InventoryItem)
	{
		StackCountText->SetText(FText::AsNumber(InventoryItem->GetStackCount()));
	}
}

void UInventoryItemWidget::StartCooldown(float CooldownDuration, float TimeRemaining)
{
	CooldownTimeRemaining = TimeRemaining;
	CooldownTimeDuration = CooldownDuration;
	GetWorld()->GetTimerManager().SetTimer(CooldownDurationTimerHandle, this, &UInventoryItemWidget::CooldownFinished, CooldownTimeRemaining);
	GetWorld()->GetTimerManager().SetTimer(CooldownUpdateTimerHandle, this, &UInventoryItemWidget::UpdateCooldown, CooldownUpdateInterval, true);

	CooldownCountText->SetVisibility(ESlateVisibility::Visible);
}

void UInventoryItemWidget::CooldownFinished()
{
	GetWorld()->GetTimerManager().ClearTimer(CooldownUpdateTimerHandle);
	CooldownCountText->SetVisibility(ESlateVisibility::Hidden);

	if (GetItemIcon())
	{
		GetItemIcon()->GetDynamicMaterial()->SetScalarParameterValue(CooldownAmountDynamicMaterialParamName, 1.f);
	}
}

void  UInventoryItemWidget::UpdateCooldown()
{
	CooldownTimeRemaining -= CooldownUpdateInterval;
	float CooldownAmount = 1.f - CooldownTimeRemaining / CooldownTimeDuration;
	CooldownDisplayFormattingOptions.MaximumFractionalDigits = CooldownTimeRemaining > 1.f ? 0 : 2;
	CooldownCountText->SetText(FText::AsNumber(CooldownTimeRemaining, &CooldownDisplayFormattingOptions));

	if (GetItemIcon())
	{
		GetItemIcon()->GetDynamicMaterial()->SetScalarParameterValue(CooldownAmountDynamicMaterialParamName, CooldownAmount);
	}
}

void UInventoryItemWidget::ClearCooldown()
{
	CooldownFinished();
}

void UInventoryItemWidget::SetIcon(UTexture2D* IconTexture)
{
	if (GetItemIcon())
	{
		GetItemIcon()->GetDynamicMaterial()->SetTextureParameterValue(IconTextureDynamicParamName, IconTexture);
		return;
	}

	Super::SetIcon(IconTexture);
}
