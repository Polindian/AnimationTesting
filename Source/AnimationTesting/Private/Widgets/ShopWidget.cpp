// Christopher Naglik All Rights Reserved

#include "Widgets/ShopWidget.h"
#include "Widgets/ItemWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Inventory/InventoryComponent.h"
#include "Framework/CAssetManager.h"
#include "Player/ChrisPlayerController.h"

void UShopWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetIsFocusable(true);
    LoadShopItems();

    if (APawn* OwnerPawn = GetOwningPlayerPawn())
    {
        OwnerInventoryComponent = OwnerPawn->FindComponentByClass<UInventoryComponent>();
    }

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
        }
    }
}

void UShopWidget::OnItemPurchaseRequested(const UPA_ShopItem* Item)
{
    if (OwnerInventoryComponent && Item)
    {
        OwnerInventoryComponent->TryPurchase(Item);
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

void UShopWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!TimerText) return;

    float Remaining = FMath::Max(0.f, EndTime - GetWorld()->GetTimeSeconds());
    int32 Minutes = (int32)(Remaining / 60.f);
    int32 Seconds = (int32)(Remaining) % 60;
    TimerText->SetText(FText::FromString(FString::Printf(TEXT("%d:%02d"), Minutes, Seconds)));
}

void UShopWidget::OnContinueClicked()
{
    AChrisPlayerController* PC = Cast<AChrisPlayerController>(GetOwningPlayer());
    if (PC)
    {
        PC->Server_VoteContinue();
    }
}
