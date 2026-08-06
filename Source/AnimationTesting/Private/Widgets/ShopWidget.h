// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "ShopWidget.generated.h"

class UPA_ShopItem;
class UItemWidget;
class UInventoryComponent;
class UItemToolTip;
class UShopCategoryWidget;      
class UMenuButtonWidget;

UCLASS()
class UShopWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void StartTimer(float Duration);
    void FocusDefaultItem();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:

    bool bShopClosed = false;
    bool bSuppressFocusSound = false;

    UFUNCTION()
    void OnContinueClicked();

    void OnItemPurchaseRequested(const UPA_ShopItem* Item);

    float EndTime = 0.f;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TimerText;

    UPROPERTY(meta = (BindWidget))
    class UMenuButtonWidget* ContinueButton;

    // Skills
    UPROPERTY(meta = (BindWidget))
    UItemWidget* Skill_Assassin1;
    UPROPERTY(meta = (BindWidget))
    UItemWidget* Skill_Assassin2;
    UPROPERTY(meta = (BindWidget))
    UItemWidget* Skill_Assassin3;
    UPROPERTY(meta = (BindWidget))
    UItemWidget* Skill_Gambler1;
    UPROPERTY(meta = (BindWidget))
    UItemWidget* Skill_Gambler2;
    UPROPERTY(meta = (BindWidget))
    UItemWidget* Skill_Gambler3;
    UPROPERTY(meta = (BindWidget))
    UItemWidget* Skill_Tank1;
    UPROPERTY(meta = (BindWidget))
    UItemWidget* Skill_Tank2;
    UPROPERTY(meta = (BindWidget))
    UItemWidget* Skill_Tank3;
    UPROPERTY(meta = (BindWidget))
    UItemWidget* Skill_Magician1;
    UPROPERTY(meta = (BindWidget))
    UItemWidget* Skill_Magician2;
    UPROPERTY(meta = (BindWidget))
    UItemWidget* Skill_Magician3;

    // Consumables
    UPROPERTY(meta = (BindWidget))
    UItemWidget* ElixirOfLife;
    UPROPERTY(meta = (BindWidget))
    UItemWidget* BloodSerum;
    UPROPERTY(meta = (BindWidget))
    UItemWidget* WardensPhial;
    UPROPERTY(meta = (BindWidget))
    UItemWidget* Quicksilver;
    UPROPERTY(meta = (BindWidget))
    UItemWidget* Nightflare;

    // Ability Upgrades
    UPROPERTY(meta = (BindWidget))
    UItemWidget* Bonebreaker;
    UPROPERTY(meta = (BindWidget))
    UItemWidget* Shockwave;    
    UPROPERTY(meta = (BindWidget))
    UItemWidget* Scorched;
    UPROPERTY(meta = (BindWidget))
    UItemWidget* Deadeye;

    // Stock
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* ElixirOfLifeStockText;
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* BloodSerumStockText;
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* WardensPhialStockText;
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* QuicksilverStockText;
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* NightflareStockText;

    void LoadShopItems();
    void ShopItemLoadFinished();
    void BindWidgetPurchase(UItemWidget* Widget);

    void DecrementStockForItem(const UPA_ShopItem* Item);
    void RestoreShopState();

    UPROPERTY()
    UInventoryComponent* OwnerInventoryComponent;

    void UpdateSkillLockStates();
    void OnSkillPurchasedCallback(const UPA_ShopItem* Item);

    void UpdateAbilityUpgradeStates();

    // ── Category Tooltips ──────────────────────────────────────────

    UPROPERTY(meta = (BindWidget))
    UShopCategoryWidget* CategoryHeader_Assassin;
    UPROPERTY(meta = (BindWidget))
    UShopCategoryWidget* CategoryHeader_Gambler;
    UPROPERTY(meta = (BindWidget))
    UShopCategoryWidget* CategoryHeader_Tank;
    UPROPERTY(meta = (BindWidget))
    UShopCategoryWidget* CategoryHeader_Magician;
    UPROPERTY(meta = (BindWidget))
    UShopCategoryWidget* AbilityUpgradesHeader;
    UPROPERTY(meta = (BindWidget))
    UShopCategoryWidget* ConsumablesHeader;


    // ── Branch Fill System ──────────────────────────────────────────

// Branch images connecting skill tiers (set in WBP editor).
    UPROPERTY(meta = (BindWidget))
    class UImage* Branch_Assassin_0to1;
    UPROPERTY(meta = (BindWidget))
    class UImage* Branch_Assassin_1to2;
    UPROPERTY(meta = (BindWidget))
    class UImage* Branch_Assassin_2to3;
    UPROPERTY(meta = (BindWidget))
    class UImage* Branch_Gambler_0to1;
    UPROPERTY(meta = (BindWidget))
    class UImage* Branch_Gambler_1to2;
    UPROPERTY(meta = (BindWidget))
    class UImage* Branch_Gambler_2to3;
    UPROPERTY(meta = (BindWidget))
    class UImage* Branch_Tank_0to1;
    UPROPERTY(meta = (BindWidget))
    class UImage* Branch_Tank_1to2;
    UPROPERTY(meta = (BindWidget))
    class UImage* Branch_Tank_2to3;
    UPROPERTY(meta = (BindWidget))
    class UImage* Branch_Magician_0to1;
    UPROPERTY(meta = (BindWidget))
    class UImage* Branch_Magician_1to2;
    UPROPERTY(meta = (BindWidget))
    class UImage* Branch_Magician_2to3;

    // The parent material created in Step 1. Set this in the WBP class defaults.
    UPROPERTY(EditDefaultsOnly, Category = "Branch Fill")
    UMaterialInterface* BranchFillMaterial;

    // How long the fill animation takes (seconds).
    UPROPERTY(EditDefaultsOnly, Category = "Branch Fill")
    float BranchFillDuration = 1.0f;

    // Category colors for the branch fill. Order: Assassin, Gambler, Tank, Magician.
    UPROPERTY(EditDefaultsOnly, Category = "Branch Fill")
    TArray<FLinearColor> CategoryColors;

    // Tracks one active branch-fill animation.
    struct FBranchFillAnim
    {
        UMaterialInstanceDynamic* DMI = nullptr;
        float Elapsed = 0.f;
        int32 CategoryIndex = -1;
        int32 TierIndex = -1;       // tier that was just purchased (0-based)
    };

    // All currently-playing branch fill animations.
    TArray<FBranchFillAnim> ActiveBranchAnims;

    // Runtime DMIs for each branch (parallel to the branch image arrays).
    // [CategoryIndex][0 = 1to2, 1 = 2to3]
    TArray<TArray<UMaterialInstanceDynamic*>> BranchDMIs;

    // Helpers
    void InitBranchMaterials();
    void StartBranchFillAnim(int32 CategoryIndex, int32 TierIndex);
    void TickBranchAnims(float DeltaTime);
    void OnBranchFillComplete(const FBranchFillAnim& Anim);

    // Finds category/tier index for a purchased shop item. Returns false if not a skill.
    bool FindSkillIndices(const UPA_ShopItem* Item, int32& OutCategory, int32& OutTier) const;


    /******************************/
    /*         Navigation         */
    /******************************/

    UPROPERTY(meta = (BindWidget))
    class UCanvasPanel* TooltipCanvas;

    UPROPERTY(meta = (BindWidget))
    class UImage* TooltipImage;

    UPROPERTY(EditDefaultsOnly, Category = "Tooltip")
    FVector2D TooltipOffset = FVector2D(20.f, 0.f);

    void HandleItemFocusChanged(UItemWidget* Item, bool bFocused);
    void HandleCategoryFocusChanged(UShopCategoryWidget* Category, bool bFocused);
    void ShowTooltipNextTo(UWidget* Target, UTexture2D* Texture);
    void HideTooltip();

    // All focusable shop entries, gathered once for nav wiring and default focus
    TArray<UItemWidget*> AllItemWidgets;

    static void SetNav(UWidget* From, UWidget* Up, UWidget* Down, UWidget* Left, UWidget* Right);
    void WireShopNavigation();
};


