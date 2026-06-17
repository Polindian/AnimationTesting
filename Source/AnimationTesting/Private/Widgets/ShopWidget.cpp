// Christopher Naglik All Rights Reserved

#include "Widgets/ShopWidget.h"
#include "Widgets/ItemWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Inventory/InventoryComponent.h"
#include "Widgets/ItemToolTip.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/CAssetManager.h"
#include "Player/ChrisPlayerController.h"

void UShopWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetIsFocusable(true);
    LoadShopItems();
    InitBranchMaterials();

    if (APawn* OwnerPawn = GetOwningPlayerPawn())
    {
        OwnerInventoryComponent = OwnerPawn->FindComponentByClass<UInventoryComponent>();

        if (OwnerInventoryComponent)
        {
            OwnerInventoryComponent->OnSkillPurchased.AddUObject(this, &UShopWidget::OnSkillPurchasedCallback);
        }
    }

    SetupCategoryTooltips();

    if (ContinueButton)
    {
        ContinueButton->OnClicked.AddDynamic(this, &UShopWidget::OnContinueClicked);
    }
}

void UShopWidget::BindWidgetPurchase(UItemWidget* Widget)
{
    if (Widget)
    {
        Widget->OnItemPurchaseRequested.AddUObject(this, &UShopWidget::OnItemPurchaseRequested);
    }
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
    if (bShopClosed)
        return;
    
    if (OwnerInventoryComponent && Item)
    {
        bool bSuccess = OwnerInventoryComponent->TryPurchase(Item);
        if (bSuccess && Item->GetIsConsumable())
        {
            DecrementStockForItem(Item);
        }
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

void UShopWidget::DecrementStockForItem(const UPA_ShopItem* Item)
{
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
            Pair.Key->DecrementStock();
            if (Pair.Value)
            {
                Pair.Value->SetText(FText::AsNumber(Pair.Key->GetStock()));
            }
            break;
        }
    }
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

    // Restore consumable stock based on how many the player already owns
    for (auto& Pair : ConsumableSlots)
    {
        if (Pair.Key && Pair.Key->GetShopItem() && Pair.Key->GetShopItem()->GetIsConsumable())
        {
            int32 Owned = OwnerInventoryComponent->GetConsumableCount(Pair.Key->GetShopItem());
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
        UpdateAbilityUpgradeStates();
        return;
    }

    StartBranchFillAnim(CatIdx, TierIdx);

    
}

void UShopWidget::UpdateAbilityUpgradeStates()
{
    TArray<UItemWidget*> Upgrades = {
       Bonebreaker, Shockwave, Scorched, Deadeye
    };

    for (UItemWidget* UpgradeWidget : Upgrades)
    {
        if (!UpgradeWidget || !UpgradeWidget->GetShopItem())
            continue;

        const bool bPurchased = OwnerInventoryComponent->HasPurchased(UpgradeWidget->GetShopItem());
        // No lock system for upgrades — always available, never locked.
        UpgradeWidget->SetSkillState(bPurchased, /*bLocked=*/ false);
    }
}

void UShopWidget::SetupCategoryTooltips()
{
    if (!CategoryToolTipClass) return;

    // Pair each category header widget with its tooltip texture.
    TArray<TPair<UWidget*, UTexture2D*>> Categories = {
        { CategoryHeader_Assassin,  AssassinTooltipTexture  },
        { CategoryHeader_Gambler,   GamblerTooltipTexture   },
        { CategoryHeader_Tank,      TankTooltipTexture      },
        { CategoryHeader_Magician,  MagicianTooltipTexture  },
        { AbilityUpgradesHeader,    AbilityUpgradesTooltipTexture }, 
        { ConsumablesHeader,        ConsumablesTooltipTexture     }
    };

    for (auto& Pair : Categories)
    {
        if (!Pair.Key || !Pair.Value) continue;

        UItemToolTip* Tooltip = CreateWidget<UItemToolTip>(GetOwningPlayer(), CategoryToolTipClass);
        if (Tooltip)
        {
            Tooltip->SetTooltipImage(Pair.Value);

            // Apply custom size for the non-standard aspect ratio tooltips.
            if (Pair.Key == AbilityUpgradesHeader)
            {
                Tooltip->SetTooltipSize(AbilityUpgradesTooltipSize);
            }
            else if (Pair.Key == ConsumablesHeader)
            {
                Tooltip->SetTooltipSize(ConsumablesTooltipSize);
            }

            Pair.Key->SetToolTip(Tooltip);
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
    if (SkillUnlockSound)
    {
        UGameplayStatics::PlaySound2D(this, SkillUnlockSound);
    }

    // Now update all skill lock states 
    UpdateSkillLockStates();
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
            bShopClosed = true;
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
    bShopClosed = true;
    
    AChrisPlayerController* PC = Cast<AChrisPlayerController>(GetOwningPlayer());
    if (PC)
    {
        PC->Server_VoteContinue();
    }
}