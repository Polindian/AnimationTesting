// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopWidget.generated.h"

class UPA_ShopItem;
class UItemWidget;
class UInventoryComponent;

UCLASS()
class UShopWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void StartTimer(float Duration);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    UFUNCTION()
    void OnContinueClicked();

    void OnItemPurchaseRequested(const UPA_ShopItem* Item);

    float EndTime = 0.f;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TimerText;

    UPROPERTY(meta = (BindWidget))
    class UButton* ContinueButton;

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
};
