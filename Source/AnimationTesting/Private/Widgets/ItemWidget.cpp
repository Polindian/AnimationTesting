// Christopher Naglik All Rights Reserved

#include "Widgets/ItemWidget.h"
#include "Widgets/ItemToolTip.h"
#include "Components/Image.h"
#include "Inventory/PA_ShopItem.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Audio/ChrisAudioSubsystem.h"
#include "Audio/ChrisGameplayTags.h"

void UItemWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetIsFocusable(true);

    if (LockIcon)
    {
        LockIcon->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (ShimmerImage)
    {
        ShimmerImage->SetVisibility(ESlateVisibility::Collapsed);
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
    UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this);

    // Locked skills (previous tier not bought) cannot be purchased.
    if (bIsLocked)
    {
        if (Audio) { Audio->Play2D(ChrisGameplayTags::Audio_UI_Reject); }
        return;
    }

    // Already-owned skills (non-consumables) cannot be re-bought.
    if (bIsPurchased && ShopItem && !ShopItem->GetIsConsumable())
    {
        if (Audio) { Audio->Play2D(ChrisGameplayTags::Audio_UI_Reject); }
        return;
    }

    // Existing consumable out-of-stock guard.
    if (Stock <= 0 && ShopItem && ShopItem->GetIsConsumable())
    {
        if (Audio) { Audio->Play2D(ChrisGameplayTags::Audio_UI_Reject); }
        return;
    }

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
    if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
    {
        return FReply::Unhandled();
    }

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

void UItemWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
    // Route mouse through focus so all three devices share one highlight path
    FocusItem();
}

void UItemWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
    Super::NativeOnAddedToFocusPath(InFocusEvent);

    UE_LOG(LogTemp, Warning, TEXT("[Shop] Focus arrived: %s | HasKeyboardFocus=%d"),
        *GetName(), HasKeyboardFocus() ? 1 : 0);

    // Scale grows from the widget's centre (default pivot 0.5,0.5)
    SetRenderScale(FVector2D(FocusScale, FocusScale));

    OnItemFocusChanged.Broadcast(this, true);
}

void UItemWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
    Super::NativeOnRemovedFromFocusPath(InFocusEvent);

    SetRenderScale(FVector2D(1.f, 1.f));

    OnItemFocusChanged.Broadcast(this, false);
}

FReply UItemWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();
    if (Key == EKeys::Enter || Key == EKeys::Gamepad_FaceButton_Bottom || Key == EKeys::Virtual_Accept)
    {
        OnItemClicked();   // same path as a mouse click, including all its guards
        return FReply::Handled();
    }
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}


void UItemWidget::SetShimmerActive(bool bActive, FLinearColor Colour, bool bDiamond)
{
    UE_LOG(LogTemp, Warning, TEXT("[Shimmer] %s active=%d image=%d ring=%d diamond=%d"),
        *GetName(), bActive, ShimmerImage != nullptr,
        ShimmerMaterialRing != nullptr, ShimmerMaterialDiamond != nullptr);
    
    if (!ShimmerImage) return;

    if (!bActive)
    {
        ShimmerImage->SetVisibility(ESlateVisibility::Collapsed);
        return;
    }

    if (!ShimmerMID || bShimmerIsDiamond != bDiamond)
    {
        UMaterialInterface* Source = bDiamond ? ShimmerMaterialDiamond : ShimmerMaterialRing;
        if (!Source) return;

        ShimmerMID = UMaterialInstanceDynamic::Create(Source, this);
        bShimmerIsDiamond = bDiamond;

        FSlateBrush Brush;
        Brush.SetResourceObject(ShimmerMID);
        Brush.DrawAs = ESlateBrushDrawType::Image;
        ShimmerImage->SetBrush(Brush);          
    }

   
    const float Scale = bDiamond ? ShimmerScaleDiamond : ShimmerScaleRing;
    ShimmerImage->SetRenderScale(FVector2D(Scale, Scale));

    ShimmerMID->SetVectorParameterValue(FName("GlowColour"), Colour);
    ShimmerImage->SetVisibility(ESlateVisibility::HitTestInvisible);
}
