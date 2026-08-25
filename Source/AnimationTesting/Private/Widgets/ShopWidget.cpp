// Christopher Naglik All Rights Reserved

#include "Widgets/ShopWidget.h"
#include "Widgets/ItemWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Widgets/ShopCategoryWidget.h"
#include "Widgets/MenuButtonWidget.h"
#include "Framework/Application/SlateApplication.h"
#include "Inventory/InventoryComponent.h"
#include "Widgets/ItemToolTip.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/CAssetManager.h"
#include "Player/ChrisPlayerController.h"
#include "Audio/ChrisAudioSubsystem.h"
#include "Audio/ChrisGameplayTags.h"

void UShopWidget::NativeConstruct()
{
    Super::NativeConstruct();
    LoadShopItems();
    InitBranchMaterials();
    BuildSkillWidgetGrid();

    if (APawn* OwnerPawn = GetOwningPlayerPawn())
    {
        OwnerInventoryComponent = OwnerPawn->FindComponentByClass<UInventoryComponent>();

        if (OwnerInventoryComponent)
        {
            OwnerInventoryComponent->OnSkillPurchased.AddUObject(this, &UShopWidget::OnSkillPurchasedCallback);
            OwnerInventoryComponent->OnSkillPurchased.AddUObject(this, &UShopWidget::OnSkillPurchasedCallback);
            OwnerInventoryComponent->OnPurchaseFailed.AddUObject(this, &UShopWidget::HandlePurchaseFailed);
            OwnerInventoryComponent->OnConsumablePurchaseCountChanged.AddUObject(this, &UShopWidget::HandleConsumableCountChanged);
        }
    }

    if (ContinueButton)
    {
        ContinueButton->OnMenuButtonClicked.AddDynamic(this, &UShopWidget::OnContinueClicked);
    }

    // Categories aren't purchasable — they only announce their tooltip on focus
    UShopCategoryWidget* Categories[] = {
        CategoryHeader_Assassin, CategoryHeader_Gambler, CategoryHeader_Tank,
        CategoryHeader_Magician, AbilityUpgradesHeader, ConsumablesHeader
    };
    for (UShopCategoryWidget* Category : Categories)
    {
        if (Category)
        {
            Category->OnCategoryFocusChanged.AddUObject(this, &UShopWidget::HandleCategoryFocusChanged);
        }
    }
}

void UShopWidget::BindWidgetPurchase(UItemWidget* Widget)
{
    if (!Widget) return;

    Widget->OnItemPurchaseRequested.AddUObject(this, &UShopWidget::OnItemPurchaseRequested);
    Widget->OnItemFocusChanged.AddUObject(this, &UShopWidget::HandleItemFocusChanged);
    AllItemWidgets.AddUnique(Widget);
}

void UShopWidget::ShopItemLoadFinished()
{
    TArray<const UPA_ShopItem*> ShopItems;
    UCAssetManager::Get().GetLoadedShopItems(ShopItems);

    TMap<FName, UItemWidget*> WidgetMap;

    //Skills
    WidgetMap.Add(FName("DA_BladedEdge"), Skill_Assassin1);
    WidgetMap.Add(FName("DA_Fleetfoot"), Skill_Assassin2);
    WidgetMap.Add(FName("DA_StealthStrike"), Skill_Assassin3);
    WidgetMap.Add(FName("DA_DoubleDown"), Skill_Gambler1);
    WidgetMap.Add(FName("DA_StackedOdds"), Skill_Gambler2);
    WidgetMap.Add(FName("DA_LoadedDice"), Skill_Gambler3);
    WidgetMap.Add(FName("DA_IronClad"), Skill_Tank1);
    WidgetMap.Add(FName("DA_Juggernaut"), Skill_Tank2);
    WidgetMap.Add(FName("DA_Bulwark"), Skill_Tank3);
    WidgetMap.Add(FName("DA_StasisWard"), Skill_Magician1);
    WidgetMap.Add(FName("DA_ArcaneAegis"), Skill_Magician2);
    WidgetMap.Add(FName("DA_Dominion"), Skill_Magician3);

    //Consumables
    WidgetMap.Add(FName("DA_ElixirOfLife"), ElixirOfLife);
    WidgetMap.Add(FName("DA_BloodSerum"), BloodSerum);
    WidgetMap.Add(FName("DA_WardensPhial"), WardensPhial);
    WidgetMap.Add(FName("DA_Quicksilver"), Quicksilver);
    WidgetMap.Add(FName("DA_Nightflare"), Nightflare);

    //Ability Upgrades
    WidgetMap.Add(FName("DA_Bonebreaker"), Bonebreaker);
    WidgetMap.Add(FName("DA_Shockwave"), Shockwave);
    WidgetMap.Add(FName("DA_Scorched"), Scorched);
    WidgetMap.Add(FName("DA_Deadeye"), Deadeye);

    for (const UPA_ShopItem* ShopItem : ShopItems)
    {
        if (!ShopItem) continue;

        FName AssetName = ShopItem->GetFName();
        UItemWidget** FoundWidget = WidgetMap.Find(AssetName);
        if (FoundWidget && *FoundWidget)
        {
            (*FoundWidget)->SetShopItem(ShopItem);
            (*FoundWidget)->SetIcon(ShopItem->GetIcon());
            (*FoundWidget)->SetTooltipTexture(ShopItem->GetTooltipIcon());
            BindWidgetPurchase(*FoundWidget);

            if (ShopItem->GetIsConsumable())
            {
                (*FoundWidget)->SetStock(ShopItem->GetMaxStackCount());
            }
        }
    }

    RestoreShopState();
}

void UShopWidget::OnItemPurchaseRequested(const UPA_ShopItem* Item)
{
    UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this);

    // Closed during the fade-in and after the timer or Continue — the server
    // would reject these anyway, so reject them here where we can say so
    if (!bPurchasingOpen)
    {
        if (Audio) { Audio->Play2D(ChrisGameplayTags::Audio_UI_Reject); }
        return;
    }

    if (!OwnerInventoryComponent || !Item) return;

    const bool bSuccess = OwnerInventoryComponent->TryPurchase(Item);

    if (!bSuccess)
    {
        // One reject for all failure reasons: not enough souls, no stock, already owned
        if (Audio) { Audio->Play2D(ChrisGameplayTags::Audio_UI_Reject); }
        return;
    }

    if (Item->GetIsConsumable())
    {
        // Stock is NOT touched here — HandleConsumableCountChanged owns the display
        // and gets an absolute count from the server. On a listen server that
        // arrives synchronously, so a decrement here would double up
        if (Audio) { Audio->Play2D(ChrisGameplayTags::Audio_UI_Shop_Unlock_Consumable); }
        return;
    }

    // Not a consumable: FindSkillIndices separates skills from ability upgrades
    int32 CatIdx, TierIdx;
    const bool bIsSkill = FindSkillIndices(Item, CatIdx, TierIdx);

    if (Audio)
    {
        Audio->Play2D(bIsSkill
            ? ChrisGameplayTags::Audio_UI_Shop_Unlock_Skill
            : ChrisGameplayTags::Audio_UI_Shop_Unlock_Upgrade);
    }
}


void UShopWidget::LoadShopItems()
{
    UCAssetManager::Get().LoadShopItems(FStreamableDelegate::CreateUObject(this, &UShopWidget::ShopItemLoadFinished));
}

void UShopWidget::StartTimer(float Duration)
{
    EndTime = GetWorld()->GetTimeSeconds() + Duration;
}



void UShopWidget::RestoreShopState()
{
    if (!OwnerInventoryComponent) return;

    TArray<TPair<UItemWidget*, UTextBlock*>> ConsumableSlots = {
        {ElixirOfLife, ElixirOfLifeStockText},
        {BloodSerum, BloodSerumStockText},
        {WardensPhial, WardensPhialStockText},
        {Quicksilver, QuicksilverStockText},
        {Nightflare, NightflareStockText}
    };

    // Restore consumable stock based on how many the player already bought
    for (auto& Pair : ConsumableSlots)
    {
        if (Pair.Key && Pair.Key->GetShopItem() && Pair.Key->GetShopItem()->GetIsConsumable())
        {
            int32 Owned = OwnerInventoryComponent->GetConsumablePurchaseCount(Pair.Key->GetShopItem());
            int32 MaxStock = Pair.Key->GetShopItem()->GetMaxStackCount();
            int32 Remaining = FMath::Max(0, MaxStock - Owned);
            Pair.Key->SetStock(Remaining);
            if (Pair.Value)
            {
                Pair.Value->SetText(FText::AsNumber(Remaining));
            }
        }
    }

    UpdateSkillLockStates();
    UpdateAbilityUpgradeStates();

    // Pre-fill branches for already-purchased skills (e.g. reconnect, re-open shop).
    if (BranchDMIs.Num() > 0 && OwnerInventoryComponent)
    {
        TArray<TArray<UItemWidget*>> Categories = {
            { Skill_Assassin1, Skill_Assassin2, Skill_Assassin3 },
            { Skill_Gambler1,  Skill_Gambler2,  Skill_Gambler3  },
            { Skill_Tank1,     Skill_Tank2,     Skill_Tank3     },
            { Skill_Magician1, Skill_Magician2, Skill_Magician3 }
        };

        for (int32 Cat = 0; Cat < Categories.Num(); ++Cat)
        {
            for (int32 Tier = 0; Tier < Categories[Cat].Num() - 1; ++Tier)
            {
                UItemWidget* SkillWidget = Categories[Cat][Tier];
                if (SkillWidget && SkillWidget->GetShopItem() &&
                    OwnerInventoryComponent->HasPurchased(SkillWidget->GetShopItem()))
                {
                    // This tier is already owned — snap its branch to fully filled.
                    if (BranchDMIs.IsValidIndex(Cat) && BranchDMIs[Cat].IsValidIndex(Tier) && BranchDMIs[Cat][Tier])
                    {
                        BranchDMIs[Cat][Tier]->SetScalarParameterValue(FName("FillAmount"), 1.f);
                    }
                }
            }

            // Shimmer on every owned tier, including the last one
            for (int32 Tier = 0; Tier < Categories[Cat].Num(); ++Tier)
            {
                UItemWidget* SkillWidget = Categories[Cat][Tier];
                if (SkillWidget && SkillWidget->GetShopItem() && OwnerInventoryComponent->HasPurchased(SkillWidget->GetShopItem()))
                {
                    ApplySkillShimmer(Cat, Tier);
                }
            }
        }
    }
}

void UShopWidget::UpdateSkillLockStates()
{
    TArray<TArray<UItemWidget*>> Categories = {
        { Skill_Assassin1, Skill_Assassin2, Skill_Assassin3 },
        { Skill_Gambler1,  Skill_Gambler2,  Skill_Gambler3  },
        { Skill_Tank1,     Skill_Tank2,     Skill_Tank3     },
        { Skill_Magician1, Skill_Magician2, Skill_Magician3 }
    };

    for (const TArray<UItemWidget*>& Tiers : Categories)
    {
        // Tier 1 has no prerequisite, so it always starts unlocked.
        bool bPreviousPurchased = true;

        for (UItemWidget* SkillWidget : Tiers)
        {
            if (!SkillWidget || !SkillWidget->GetShopItem())
                continue;

            const bool bPurchased = OwnerInventoryComponent->HasPurchased(SkillWidget->GetShopItem());

            // Locked if the previous tier in this category isn't owned yet.
            const bool bLocked = !bPreviousPurchased;

            SkillWidget->SetSkillState(bPurchased, bLocked);

            // The NEXT tier's unlock depends on whether THIS tier is now owned.
            bPreviousPurchased = bPurchased;
        }
    }
}

void UShopWidget::OnSkillPurchasedCallback(const UPA_ShopItem* Item)
{
    int32 CatIdx, TierIdx;
    if (!FindSkillIndices(Item, CatIdx, TierIdx))
    {
        // Not a skill (consumable) — just refresh states normally.
        UpdateSkillLockStates();
        UpdateAbilityUpgradeStates(Item);
        return;
    }

    StartBranchFillAnim(CatIdx, TierIdx);
}

void UShopWidget::UpdateAbilityUpgradeStates(const UPA_ShopItem* JustPurchasedItem)
{
    if (!OwnerInventoryComponent) return;

    TArray<UItemWidget*> Upgrades = { Bonebreaker, Shockwave, Scorched, Deadeye };

    for (UItemWidget* UpgradeWidget : Upgrades)
    {
        if (!UpgradeWidget || !UpgradeWidget->GetShopItem()) continue;

        const bool bPurchased = OwnerInventoryComponent->HasPurchased(UpgradeWidget->GetShopItem());
        UpgradeWidget->SetSkillState(bPurchased, /*bLocked=*/ false);

        if (!bPurchased)
        {
            UpgradeWidget->SetShimmerActive(false);
            continue;
        }

        UpgradeWidget->PrepareShimmer(FLinearColor::White, true);

        // Only the item just bought spins; everything else reveals immediately
        if (JustPurchasedItem && UpgradeWidget->GetShopItem() == JustPurchasedItem)
        {
            UpgradeWidget->PlayPurchaseSpin();
        }
        else
        {
            UpgradeWidget->RevealShimmer();
        }
    }
}

void UShopWidget::InitBranchMaterials()
{
    if (!BranchFillMaterial) return;

    // Build parallel arrays of branch images, matching CategoryColors order.
    TArray<TArray<UImage*>> BranchImages = {
        { Branch_Assassin_0to1, Branch_Assassin_1to2,  Branch_Assassin_2to3  },
        { Branch_Gambler_0to1,  Branch_Gambler_1to2,   Branch_Gambler_2to3   },
        { Branch_Tank_0to1,     Branch_Tank_1to2,      Branch_Tank_2to3      },
        { Branch_Magician_0to1, Branch_Magician_1to2,  Branch_Magician_2to3  }
    };

    // Default colors if the designer hasn't set them yet.
    if (CategoryColors.Num() < 4)
    {
        CategoryColors = {
            FLinearColor(0.8f, 0.1f, 0.1f, 1.f),  // Assassin – red
            FLinearColor(0.5f, 0.1f, 0.8f, 1.f),  // Gambler  – purple
            FLinearColor(0.1f, 0.7f, 0.2f, 1.f),  // Tank     – green
            FLinearColor(0.1f, 0.4f, 0.9f, 1.f)   // Magician – blue
        };
    }

    BranchDMIs.SetNum(BranchImages.Num());

    for (int32 Cat = 0; Cat < BranchImages.Num(); ++Cat)
    {
        BranchDMIs[Cat].SetNum(BranchImages[Cat].Num());

        for (int32 Br = 0; Br < BranchImages[Cat].Num(); ++Br)
        {
            UImage* BranchImg = BranchImages[Cat][Br];
            if (!BranchImg) continue;

            // Create a unique DMI for this branch.
            UMaterialInstanceDynamic* DMI = UMaterialInstanceDynamic::Create(BranchFillMaterial, this);

            // Pull the texture already assigned to this image in the WBP editor.
            UObject* ExistingResource = BranchImg->GetBrush().GetResourceObject();
            if (UTexture2D* BranchTex = Cast<UTexture2D>(ExistingResource))
            {
                DMI->SetTextureParameterValue(FName("BranchTexture"), BranchTex);
            }

            // Start fully grey (unfilled).
            DMI->SetScalarParameterValue(FName("FillAmount"), 0.f);
            DMI->SetVectorParameterValue(FName("FillColor"), CategoryColors[Cat]);

            // Apply the material as the brush.
            FSlateBrush Brush;
            Brush.SetResourceObject(DMI);
            Brush.ImageSize = BranchImg->GetBrush().ImageSize; // keep original size
            Brush.DrawAs = ESlateBrushDrawType::Image;
            BranchImg->SetBrush(Brush);

            BranchDMIs[Cat][Br] = DMI;
        }
    }
}

void UShopWidget::StartBranchFillAnim(int32 CategoryIndex, int32 TierIndex)
{
    // TierIndex is the tier that was just purchased (0-based).
   // The branch to fill is BranchDMIs[Category][TierIndex] — the branch ABOVE the purchased skill.
   // If it's the last tier, there's no branch above.
    if (!BranchDMIs.IsValidIndex(CategoryIndex)) return;
    if (!BranchDMIs[CategoryIndex].IsValidIndex(TierIndex)) return;

    UMaterialInstanceDynamic* DMI = BranchDMIs[CategoryIndex][TierIndex];
    if (!DMI) return;

    FBranchFillAnim Anim;
    Anim.DMI = DMI;
    Anim.Elapsed = 0.f;
    Anim.CategoryIndex = CategoryIndex;
    Anim.TierIndex = TierIndex;
    ActiveBranchAnims.Add(Anim);
}

void UShopWidget::TickBranchAnims(float DeltaTime)
{
    for (int32 i = ActiveBranchAnims.Num() - 1; i >= 0; --i)
    {
        FBranchFillAnim& Anim = ActiveBranchAnims[i];
        Anim.Elapsed += DeltaTime;

        float Alpha = FMath::Clamp(Anim.Elapsed / BranchFillDuration, 0.f, 1.f);
        if (Anim.DMI)
        {
            Anim.DMI->SetScalarParameterValue(FName("FillAmount"), Alpha);
        }

        if (Alpha >= 1.f)
        {
            OnBranchFillComplete(Anim);
            ActiveBranchAnims.RemoveAt(i);
        }
    }
}

void UShopWidget::OnBranchFillComplete(const FBranchFillAnim& Anim)
{
    // Now update all skill lock states 
    UpdateSkillLockStates();

    ApplySkillShimmer(Anim.CategoryIndex, Anim.TierIndex);
}

bool UShopWidget::FindSkillIndices(const UPA_ShopItem* Item, int32& OutCategory, int32& OutTier) const
{
    TArray<TArray<UItemWidget*>> Categories = {
        { Skill_Assassin1, Skill_Assassin2, Skill_Assassin3 },
        { Skill_Gambler1,  Skill_Gambler2,  Skill_Gambler3  },
        { Skill_Tank1,     Skill_Tank2,     Skill_Tank3     },
        { Skill_Magician1, Skill_Magician2, Skill_Magician3 }
    };

    for (int32 Cat = 0; Cat < Categories.Num(); ++Cat)
    {
        for (int32 Tier = 0; Tier < Categories[Cat].Num(); ++Tier)
        {
            if (Categories[Cat][Tier] && Categories[Cat][Tier]->GetShopItem() == Item)
            {
                OutCategory = Cat;
                OutTier = Tier;
                return true;
            }
        }
    }

    return false;
}

// Positions the tooltip beside whatever has focus, converting the target's
// absolute screen geometry into the tooltip canvas's local space
void UShopWidget::ShowTooltipNextTo(UWidget* Target, UTexture2D* Texture)
{
    if (!Target || !Texture || !TooltipImage || !TooltipCanvas)
    {
        HideTooltip();
        return;
    }

    TooltipImage->SetBrushFromTexture(Texture, true);

    const FGeometry TargetGeo = Target->GetCachedGeometry();
    const FGeometry CanvasGeo = TooltipCanvas->GetCachedGeometry();

    FVector2D LocalPos = CanvasGeo.AbsoluteToLocal(TargetGeo.GetAbsolutePosition());
    LocalPos.X += TargetGeo.GetLocalSize().X + TooltipOffset.X;   // to the right of the item
    LocalPos.Y += TooltipOffset.Y;

    if (UCanvasPanelSlot* TooltipSlot = Cast<UCanvasPanelSlot>(TooltipImage->Slot))
    {
        TooltipSlot->SetPosition(LocalPos);
    }

    TooltipImage->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UShopWidget::HideTooltip()
{
    if (TooltipImage) { TooltipImage->SetVisibility(ESlateVisibility::Hidden); }
}

void UShopWidget::SetNav(UWidget* From, UWidget* Up, UWidget* Down, UWidget* Left, UWidget* Right)
{
    if (!From) { return; }

    if (Up) { From->SetNavigationRuleExplicit(EUINavigation::Up, Up); }
    if (Down) { From->SetNavigationRuleExplicit(EUINavigation::Down, Down); }
    if (Left) { From->SetNavigationRuleExplicit(EUINavigation::Left, Left); }
    if (Right) { From->SetNavigationRuleExplicit(EUINavigation::Right, Right); }

    From->BuildNavigation();
}

void UShopWidget::WireShopNavigation()
{
    // MenuButtonWidget's inner button is the actual focus target
    UWidget* Continue = ContinueButton ? ContinueButton->GetMainButton() : nullptr;


    // --- Assassin ---
    SetNav(Skill_Assassin1, Skill_Assassin2, Skill_Tank1, Skill_Magician1, Skill_Gambler1);
    SetNav(Skill_Assassin2, Skill_Assassin3, Skill_Assassin1, Skill_Magician1, Skill_Gambler1);
    SetNav(Skill_Assassin3, CategoryHeader_Assassin, Skill_Assassin2, Skill_Magician1, Skill_Gambler1);

    // --- Gambler ---
    SetNav(Skill_Gambler1, Skill_Assassin1, Skill_Tank1, Skill_Magician1, Skill_Gambler2);
    SetNav(Skill_Gambler2, Skill_Assassin1, Skill_Tank1, Skill_Gambler1, Skill_Gambler3);
    SetNav(Skill_Gambler3, Skill_Assassin1, Skill_Tank1, Skill_Gambler2, CategoryHeader_Gambler);

    // --- Magician ---
    SetNav(Skill_Magician1, Skill_Assassin1, Skill_Tank1, Skill_Magician2, Skill_Gambler1);
    SetNav(Skill_Magician2, Skill_Assassin1, Skill_Tank1, Skill_Magician3, Skill_Magician1);
    SetNav(Skill_Magician3, Skill_Assassin1, Skill_Tank1, CategoryHeader_Magician, Skill_Magician2);

    // --- Tank ---
    SetNav(Skill_Tank1, Skill_Assassin1, Skill_Tank2, Skill_Magician1, Skill_Gambler1);
    SetNav(Skill_Tank2, Skill_Tank1, Skill_Tank3, Skill_Magician1, Skill_Gambler1);
    SetNav(Skill_Tank3, Skill_Tank2, CategoryHeader_Tank, Skill_Magician1, Skill_Gambler1);

    // --- Headers ---
    SetNav(CategoryHeader_Assassin, CategoryHeader_Tank, Skill_Assassin3, CategoryHeader_Magician, CategoryHeader_Gambler);
    SetNav(CategoryHeader_Gambler, AbilityUpgradesHeader, CategoryHeader_Tank, Skill_Gambler3, AbilityUpgradesHeader);
    SetNav(CategoryHeader_Tank, Skill_Tank3, CategoryHeader_Assassin, CategoryHeader_Magician, Continue);
    SetNav(CategoryHeader_Magician, CategoryHeader_Assassin, CategoryHeader_Tank, ConsumablesHeader, Skill_Magician3);
    SetNav(AbilityUpgradesHeader, Continue, Bonebreaker, CategoryHeader_Gambler, CategoryHeader_Magician);
    SetNav(ConsumablesHeader, Deadeye, ElixirOfLife, CategoryHeader_Gambler, CategoryHeader_Magician);

    // --- Ability Upgrades ---
    SetNav(Bonebreaker, AbilityUpgradesHeader, Scorched, CategoryHeader_Gambler, Shockwave);
    SetNav(Shockwave, AbilityUpgradesHeader, Deadeye, Bonebreaker, Scorched);
    SetNav(Scorched, Bonebreaker, ConsumablesHeader, CategoryHeader_Gambler, Deadeye);
    SetNav(Deadeye, Shockwave, ConsumablesHeader, Scorched, CategoryHeader_Magician);

    // --- Consumables ---
    SetNav(ElixirOfLife, ConsumablesHeader, Quicksilver, CategoryHeader_Gambler, BloodSerum);
    SetNav(BloodSerum, ConsumablesHeader, Quicksilver, ElixirOfLife, WardensPhial);
    SetNav(WardensPhial, ConsumablesHeader, Nightflare, BloodSerum, Quicksilver);
    SetNav(Quicksilver, ElixirOfLife, Continue, CategoryHeader_Gambler, Nightflare);
    SetNav(Nightflare, WardensPhial, Continue, Quicksilver, Continue);

    // --- Continue ---
    SetNav(Continue, Nightflare, AbilityUpgradesHeader, CategoryHeader_Tank, CategoryHeader_Magician);
}

void UShopWidget::BuildSkillWidgetGrid()
{
    SkillWidgetGrid.Empty();
    SkillWidgetGrid.Add({ Skill_Assassin1, Skill_Assassin2, Skill_Assassin3 });
    SkillWidgetGrid.Add({ Skill_Gambler1,  Skill_Gambler2,  Skill_Gambler3 });
    SkillWidgetGrid.Add({ Skill_Tank1,     Skill_Tank2,     Skill_Tank3 });
    SkillWidgetGrid.Add({ Skill_Magician1, Skill_Magician2, Skill_Magician3 });
}

void UShopWidget::ApplySkillShimmer(int32 Category, int32 Tier)
{
    if (!SkillWidgetGrid.IsValidIndex(Category)) return;
    if (!SkillWidgetGrid[Category].IsValidIndex(Tier)) return;
    if (!CategoryColors.IsValidIndex(Category)) return;

    if (UItemWidget* Widget = SkillWidgetGrid[Category][Tier])
    {
        Widget->SetShimmerActive(true, CategoryColors[Category], false);
    }
}

void UShopWidget::HandleItemFocusChanged(UItemWidget* Item, bool bFocused)
{
    if (bFocused)
    {
        if (!bSuppressFocusSound)
        {
            if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
            {
                Audio->Play2D(ChrisGameplayTags::Audio_UI_Navigate_Shop);
            }
        }
        ShowTooltipNextTo(Item, Item->GetTooltipTexture());
    }
    else { HideTooltip(); }
}

// Same for categories — separate handler only because the delegate types differ
void UShopWidget::HandleCategoryFocusChanged(UShopCategoryWidget* Category, bool bFocused)
{
    if (bFocused)
    {
        if (!bSuppressFocusSound)
        {
            if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
            {
                Audio->Play2D(ChrisGameplayTags::Audio_UI_Navigate_Shop);
            }
        }
        ShowTooltipNextTo(Category, Category->GetTooltipTexture());
    }
    else { HideTooltip(); }

}

void UShopWidget::FocusDefaultItem()
{
    if (!Skill_Assassin1) { return; }

    WireShopNavigation();

    // SetKeyboardFocus is synchronous, so the focus-changed broadcast lands inside this scope 
    bSuppressFocusSound = true;

    TSharedPtr<SWidget> SlateWidget = Skill_Assassin1->GetCachedWidget();
    if (SlateWidget.IsValid())
    {
        FSlateApplication::Get().SetKeyboardFocus(SlateWidget, EFocusCause::SetDirectly);
        FSlateApplication::Get().SetUserFocus(0, SlateWidget, EFocusCause::SetDirectly);
    }

    bSuppressFocusSound = false;
}

void UShopWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!TimerText) return;

    if (TimerText)
    {
        float Remaining = FMath::Max(0.f, EndTime - GetWorld()->GetTimeSeconds());
        int32 Minutes = (int32)(Remaining / 60.f);
        int32 Seconds = (int32)(Remaining) % 60;
        TimerText->SetText(FText::FromString(FString::Printf(TEXT("%d:%02d"), Minutes, Seconds)));

        if (Remaining <= 0.f)
        {
            bPurchasingOpen = false;
        }
    }

    // Advance branch fill animations.
    if (ActiveBranchAnims.Num() > 0)
    {
        TickBranchAnims(InDeltaTime);
    }
}

void UShopWidget::OnContinueClicked()
{
    bPurchasingOpen = false;
    
    AChrisPlayerController* PC = Cast<AChrisPlayerController>(GetOwningPlayer());
    if (PC)
    {
        PC->Server_VoteContinue();
    }
}

// The server refused after we'd already shown success — undo the optimistic UI
void UShopWidget::HandlePurchaseFailed(const UPA_ShopItem* Item)
{
    if (!Item) return;

    // Consumable stock needs no rollback now — it only ever moves on the
    // server's count, which never went up for a refused purchase
    if (!Item->GetIsConsumable())
    {
        // The component has already cleared its local PurchasedItems entry
        UpdateSkillLockStates();
        UpdateAbilityUpgradeStates();
    }

    if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
    {
        Audio->Play2D(ChrisGameplayTags::Audio_UI_Reject);
    }
}



// The server's purchase count is the truth — this overwrites whatever the
// optimistic click-time guess left on screen, whichever way it was wrong
void UShopWidget::HandleConsumableCountChanged(const UPA_ShopItem* Item, int32 NewCount)
{
    if (!Item) { return; }

    const int32 Remaining = FMath::Max(0, Item->GetMaxStackCount() - NewCount);

    TArray<TPair<UItemWidget*, UTextBlock*>> ConsumableSlots = {
        {ElixirOfLife, ElixirOfLifeStockText},
        {BloodSerum, BloodSerumStockText},
        {WardensPhial, WardensPhialStockText},
        {Quicksilver, QuicksilverStockText},
        {Nightflare, NightflareStockText}
    };

    for (auto& Pair : ConsumableSlots)
    {
        if (Pair.Key && Pair.Key->GetShopItem() == Item)
        {
            Pair.Key->SetStock(Remaining);
            if (Pair.Value)
            {
                Pair.Value->SetText(FText::AsNumber(Remaining));
            }
            break;
        }
    }
}