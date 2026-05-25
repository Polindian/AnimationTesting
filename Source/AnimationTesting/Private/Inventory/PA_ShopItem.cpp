// Christopher Naglik All Rights Reserved


#include "Inventory/PA_ShopItem.h"
#include "Abilities/GameplayAbility.h"

FPrimaryAssetId UPA_ShopItem::GetPrimaryAssetId() const
{
    return FPrimaryAssetId(GetShopItemAssetType(), GetFName());
}

FPrimaryAssetType UPA_ShopItem::GetShopItemAssetType()
{
    return FPrimaryAssetType("ShopItem");
}

UTexture2D* UPA_ShopItem::GetIcon() const
{
    return Icon.LoadSynchronous();
}

UTexture2D* UPA_ShopItem::GetTooltipIcon() const
{
    return TooltipIcon.LoadSynchronous();
}

UGameplayAbility* UPA_ShopItem::GetGrantedAbilityCDO() const
{
    if (GrantedAbility)
    {
        return Cast<UGameplayAbility>(GrantedAbility->GetDefaultObject());
    }

    return nullptr;
}
