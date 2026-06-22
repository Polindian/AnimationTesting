// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "Inventory/PA_ShopItem.h"
#include "CAssetManager.generated.h"

class UPA_CharacterDefinition;

/**
 * 
 */
UCLASS()
class UCAssetManager : public UAssetManager
{
	GENERATED_BODY()
	
public:
	static UCAssetManager& Get();
	void LoadShopItems(const FStreamableDelegate& LoadFinishedCallback);
	bool GetLoadedShopItems(TArray<const UPA_ShopItem*>& OutItems) const;

	// Kicks off async loading of all CharacterDefinition assets.
	// The callback delegate fires once every asset of that type is in memory.
	void LoadCharacterDefinitions(const FStreamableDelegate& LoadFinishedCallback);

	// After LoadCharacterDefinitions has completed, this is called to fill the
	// output array with pointers to all the loaded definitions.
	bool GetLoadedCharacterDefinitions(TArray<UPA_CharacterDefinition*>& LoadedCharacterDefinitions) const;

private:
	void ShopItemLoadFinished(FStreamableDelegate Callback);
};
