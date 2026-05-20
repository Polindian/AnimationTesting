// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PA_ShopItem.generated.h"

/**
 * 
 */
UCLASS()
class UPA_ShopItem : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	static FPrimaryAssetType GetShopItemAssetType();
	UTexture2D* GetIcon() const;
	UTexture2D* GetTooltipIcon() const;
	float GetPrice() const { return Price; }

	TSubclassOf<class UGameplayEffect> GetEquippedEffect() const { return EquippedEffect; }
	TSubclassOf<class UGameplayEffect> GetConsumedEffect() const { return ConsumedEffect; }
	TSubclassOf<class UGameplayAbility> GetGrantedAbility() const { return GrantedAbility; }

	bool GetIsConsumable() const { return bIsConsumable; }
	bool GetIsStackable() const { return bIsStackable; }
	int GetMaxStackCount() const { return MaxStackCount; }

private:
	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	TSoftObjectPtr<UTexture2D> TooltipIcon;

	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	float Price;

	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	bool bIsConsumable;

	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	TSubclassOf<class UGameplayEffect> EquippedEffect;

	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	TSubclassOf<class UGameplayEffect> ConsumedEffect;

	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	TSubclassOf<class UGameplayAbility> GrantedAbility;

	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	bool bIsStackable = false;

	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	int MaxStackCount = 3;
};
