// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopCategoryWidget.generated.h"

class UShopCategoryWidget;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCategoryFocusChanged, UShopCategoryWidget*, bool);

// A shop category header (ASSASSIN, TANK, CONSUMABLES...). Not purchasable —
// it exists to be navigable and to show its tooltip on focus.
UCLASS()
class UShopCategoryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    FOnCategoryFocusChanged OnCategoryFocusChanged;

    UTexture2D* GetTooltipTexture() const { return TooltipTexture; }
    void FocusCategory() { SetKeyboardFocus(); }

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
    virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;

    virtual void SynchronizeProperties() override;

private:
    UPROPERTY(meta = (BindWidgetOptional))
    class UImage* FocusTint;

    // Assigned per instance in the shop WBP — each category has its own art
    UPROPERTY(EditAnywhere, Category = "Category")
    UTexture2D* TooltipTexture;

    UPROPERTY(EditAnywhere, Category = "Category")
    float FocusScale = 1.1f;

    UPROPERTY(EditAnywhere, Category = "Category")
    UTexture2D* HeaderTexture;

    UPROPERTY(meta = (BindWidget))
    class UImage* HeaderImage;
};