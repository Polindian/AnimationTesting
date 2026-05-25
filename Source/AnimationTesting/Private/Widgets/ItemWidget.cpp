// Christopher Naglik All Rights Reserved

#include "Widgets/ItemWidget.h"
#include "Widgets/ItemToolTip.h"
#include "Components/Image.h"
#include "Inventory/PA_ShopItem.h"

void UItemWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetIsFocusable(true);
}

void UItemWidget::SetIcon(UTexture2D* IconTexture)
{
    if (ItemIcon && IconTexture)
    {
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