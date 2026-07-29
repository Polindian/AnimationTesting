// Christopher Naglik All Rights Reserved


#include "Widgets/ShopCategoryWidget.h"
#include "Components/Image.h"

void UShopCategoryWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Required for navigation to consider this widget at all
    SetIsFocusable(true);

    if (FocusTint)
    {
        FocusTint->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UShopCategoryWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
    // Route mouse through focus so all three devices share one highlight path
    FocusCategory();
}

void UShopCategoryWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
    Super::NativeOnAddedToFocusPath(InFocusEvent);

    SetRenderScale(FVector2D(FocusScale, FocusScale));
    if (FocusTint) { FocusTint->SetVisibility(ESlateVisibility::HitTestInvisible); }

    OnCategoryFocusChanged.Broadcast(this, true);
}

void UShopCategoryWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
    Super::NativeOnRemovedFromFocusPath(InFocusEvent);

    SetRenderScale(FVector2D(1.f, 1.f));
    if (FocusTint) { FocusTint->SetVisibility(ESlateVisibility::Hidden); }

    OnCategoryFocusChanged.Broadcast(this, false);
}

void UShopCategoryWidget::SynchronizeProperties()
{
    Super::SynchronizeProperties();

    if (HeaderImage && HeaderTexture)
    {
        HeaderImage->SetBrushFromTexture(HeaderTexture, false);
    }
}
