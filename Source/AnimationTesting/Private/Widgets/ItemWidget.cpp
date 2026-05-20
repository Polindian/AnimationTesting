// Christopher Naglik All Rights Reserved


#include "Widgets/ItemWidget.h"
#include "Widgets/ItemToolTip.h"
#include "Components/Image.h"

void UItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
}

void UItemWidget::SetIcon(UTexture2D* IconTexture)
{
	ItemIcon->SetBrushFromTexture(IconTexture);
}

void UItemWidget::SetTooltipTexture(UTexture2D* InTooltipTexture)
{
    TooltipTexture = InTooltipTexture;
    SetToolTipWidget();
}

UItemToolTip* UItemWidget::SetToolTipWidget()
{
    UE_LOG(LogTemp, Warning, TEXT("SetToolTipWidget called. Texture: %s, Class: %s, Player: %s"),
        TooltipTexture ? TEXT("YES") : TEXT("NULL"),
        ItemToolTipClass ? TEXT("YES") : TEXT("NULL"),
        GetOwningPlayer() ? TEXT("YES") : TEXT("NULL"));

    if (!TooltipTexture || !ItemToolTipClass || !GetOwningPlayer())
        return nullptr;

    UItemToolTip* ToolTip = CreateWidget<UItemToolTip>(GetOwningPlayer(), ItemToolTipClass);
    UE_LOG(LogTemp, Warning, TEXT("Tooltip created: %s"), ToolTip ? TEXT("YES") : TEXT("NULL"));
    if (ToolTip)
    {
        ToolTip->SetTooltipImage(TooltipTexture);
        SetToolTip(ToolTip);
    }

    return ToolTip;
}

FReply UItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        return FReply::Handled().SetUserFocus(TakeWidget());
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UItemWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (HasAnyUserFocus() && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        OnItemClicked();
        return FReply::Handled();
    }

    return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UItemWidget::OnItemClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Item clicked!"));
    if (ShopItem)
    {
        OnItemPurchaseRequested.Broadcast(ShopItem);
    }
}
