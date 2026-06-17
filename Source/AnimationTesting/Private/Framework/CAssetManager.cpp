// Christopher Naglik All Rights Reserved


#include "Framework/CAssetManager.h"

UCAssetManager& UCAssetManager::Get()
{
	UCAssetManager* Singleton = Cast<UCAssetManager>(GEngine->AssetManager.Get());

	if (Singleton)
	{
		return *Singleton;
	}

	UE_LOG(LogLoad, Fatal, TEXT("Asset Manager needs to be of the type CAssetManager"));
	return (*NewObject<UCAssetManager>());
}

void UCAssetManager::LoadShopItems(const FStreamableDelegate& LoadFinishedCallback)
{
	LoadPrimaryAssetsWithType(UPA_ShopItem::GetShopItemAssetType(), TArray<FName>(), FStreamableDelegate::CreateUObject(this, &UCAssetManager::ShopItemLoadFinished, LoadFinishedCallback));
}

bool UCAssetManager::GetLoadedShopItems(TArray<const UPA_ShopItem*>& OutItems) const
{
	TArray<UObject*> LoadedObjects;
	bool bLoaded = GetPrimaryAssetObjectList(UPA_ShopItem::GetShopItemAssetType(), LoadedObjects);

	if (bLoaded)
	{
		for(UObject* ObjectLoaded : LoadedObjects)
		{
			OutItems.Add(Cast<UPA_ShopItem>(ObjectLoaded));
		}
	}

	return bLoaded;
}

void UCAssetManager::ShopItemLoadFinished(FStreamableDelegate Callback)
{
	Callback.ExecuteIfBound();
}
