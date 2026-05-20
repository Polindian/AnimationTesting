// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemWidget.generated.h"


class UItemToolTip;
class UPA_ShopItem;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemPurchaseRequested, const UPA_ShopItem*);
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

protected:
	virtual void OnItemClicked();
	UItemToolTip* SetToolTipWidget();

private:
	UPROPERTY(meta=(BindWidget))
	class UImage* ItemIcon;

	UPROPERTY(EditAnywhere, Category = "ToolTip")
	TSubclassOf<UItemToolTip> ItemToolTipClass;

	UPROPERTY()
	UTexture2D* TooltipTexture;

	UPROPERTY()
	const UPA_ShopItem* ShopItem = nullptr;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
};
