// Christopher Naglik All Rights Reserved

#include "Widgets/ItemWidget.h"
#include "Widgets/ItemToolTip.h"
#include "Components/Image.h"
#include "Inventory/PA_ShopItem.h"
#include "Materials/MaterialInstanceDynamic.h"

void UItemWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetIsFocusable(true);

    if (LockIcon)
    {
        LockIcon->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UItemWidget::SetIcon(UTexture2D* IconTexture)
{
    if (ItemIcon && IconTexture)
    {
        ItemIcon->SetBrushFromTexture(IconTexture);
    }

    // If a skill-icon material is assigned, create a dynamic instance 
    if (SkillIconMaterial)
    {
        IconMaterialInstance = UMaterialInstanceDynamic::Create(SkillIconMaterial, this);
        IconMaterialInstance->SetTextureParameterValue(FName("IconTexture"), IconTexture);

        FSlateBrush Brush;
        Brush.SetResourceObject(IconMaterialInstance);
        Brush.ImageSize = FVector2D(65.f, 70.f);
        Brush.DrawAs = ESlateBrushDrawType::Image;
        ItemIcon->SetBrush(Brush);
    }
    else
    {
        // Fallback: consumables or items without the material use raw texture.
        ItemIcon->SetBrushFromTexture(IconTexture);
    }
}

void UItemWidget::SetTooltipTexture(UTexture2D* InTooltipTexture)
{
    TooltipTexture = InTooltipTexture;
    SetToolTipWidget();
}

UItemToolTip* UItemWidget::SetToolTipWidget()
{
    if (!ItemToolTipClass || !TooltipTexture)
    {
        return nullptr;
    }

    UItemToolTip* NewToolTip = CreateWidget<UItemToolTip>(GetOwningPlayer(), ItemToolTipClass);
    if (NewToolTip)
    {
        NewToolTip->SetTooltipImage(TooltipTexture);
        SetToolTip(NewToolTip);
    }
    return NewToolTip;
}

void UItemWidget::SetStock(int32 InStock)
{
    Stock = InStock;
}

void UItemWidget::DecrementStock()
{
    Stock = FMath::Max(0, Stock - 1);
    if (Stock <= 0)
    {
        SetColorAndOpacity(FLinearColor(0.3f, 0.3f, 0.3f, 1.f));
    }
}

void UItemWidget::OnItemClicked()
{
    // Locked skills (previous tier not bought) cannot be purchased.
    if (bIsLocked)
        return;

    // Already-owned skills (non-consumables) cannot be re-bought.
    if (bIsPurchased && ShopItem && !ShopItem->GetIsConsumable())
        return;

    // Existing consumable out-of-stock guard.
    if (Stock <= 0 && ShopItem && ShopItem->GetIsConsumable())
        return;

    if (ShopItem)
    {
        OnItemPurchaseRequested.Broadcast(ShopItem);
    }
}

FReply UItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    return FReply::Handled();
}

FReply UItemWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    OnItemClicked();
    return FReply::Handled();
}

void UItemWidget::SetSkillState(bool bPurchased, bool bLocked)
{
    bIsPurchased = bPurchased;
    bIsLocked = bLocked;

    // Lock icon is only visible while the skill is locked (previous tier not owned).
    if (LockIcon)
    {
        LockIcon->SetVisibility(bLocked ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    }

    // Drive greyscale + brightness through the dynamic material.
    if (IconMaterialInstance)
    {
        if (bPurchased)
        {
            // PURCHASED: full colour, full brightness
            IconMaterialInstance->SetScalarParameterValue(FName("Desaturation"), 0.f);
            IconMaterialInstance->SetScalarParameterValue(FName("Brightness"), 1.f);
        }
        else if (bLocked)
        {
            // LOCKED: fully greyscale, dimmed
            IconMaterialInstance->SetScalarParameterValue(FName("Desaturation"), 1.f);
            IconMaterialInstance->SetScalarParameterValue(FName("Brightness"), 0.3f);
        }
        else
        {
            // AVAILABLE (unlocked, not bought): coloured but dimmed
            IconMaterialInstance->SetScalarParameterValue(FName("Desaturation"), 0.2f);
            IconMaterialInstance->SetScalarParameterValue(FName("Brightness"), 0.2f);
        }
    }
    else if (ItemIcon)
    {
        // Fallback for widgets without the material (e.g. consumables)
        const FLinearColor Dimmed(0.3f, 0.3f, 0.3f, 1.f);
        ItemIcon->SetColorAndOpacity(bPurchased ? FLinearColor::White : Dimmed);
    }
}
