// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemWidget.generated.h"


class UItemToolTip;
class UPA_ShopItem;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemPurchaseRequested, const UPA_ShopItem*);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemFocusChanged, UItemWidget* /*Item*/, bool /*bFocused*/); 

/**
 *
 */
UCLASS()
class UItemWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void SetIcon(UTexture2D* IconTexture);
    void SetTooltipTexture(UTexture2D* InTooltipTexture);
    void SetShopItem(const UPA_ShopItem* InItem) { ShopItem = InItem; }
    const UPA_ShopItem* GetShopItem() const { return ShopItem; }

    FOnItemPurchaseRequested OnItemPurchaseRequested;

    void SetStock(int32 InStock);
    int32 GetStock() const { return Stock; }
    void DecrementStock();

private:
    int32 Stock = 0;

protected:
    virtual void OnItemClicked();

    class UImage* GetItemIcon() const { return ItemIcon; };

private:
    UPROPERTY(meta = (BindWidget))
    class UImage* ItemIcon;

    UPROPERTY(EditAnywhere, Category = "ToolTip")
    TSubclassOf<UItemToolTip> ItemToolTipClass;

    UPROPERTY()
    UTexture2D* TooltipTexture;

    UPROPERTY()
    const UPA_ShopItem* ShopItem = nullptr;

    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

/******************************/
/*   Skill Tree Lock System   */
/******************************/
public:
    void SetSkillState(bool bPurchased, bool bLocked);

private:
    UPROPERTY(meta = (BindWidgetOptional))
    class UImage* LockIcon;

    bool bIsLocked = false;
    bool bIsPurchased = false;

    // Material used for greyscale/brightness control on skill icons.
    // Set this to M_SkillIcon in the Blueprint defaults.
    UPROPERTY(EditDefaultsOnly, Category = "Skill Icon")
    UMaterialInterface* SkillIconMaterial;

    // Runtime dynamic instance so we can change parameters per-widget.
    UPROPERTY()
    UMaterialInstanceDynamic* IconMaterialInstance;


    /******************************/
    /*         Navigation         */
    /******************************/
public:
    FOnItemFocusChanged OnItemFocusChanged;

    UTexture2D* GetTooltipTexture() const { return TooltipTexture; }
    void FocusItem() { SetKeyboardFocus(); }

protected:
    virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
    virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
    // Tint overlay laid over the icon — a separate image so it never fights
    // the desaturation/brightness logic in SetSkillState
    UPROPERTY(meta = (BindWidgetOptional))
    class UImage* FocusTint;

    UPROPERTY(EditDefaultsOnly, Category = "Focus")
    float FocusScale = 1.1f;


};