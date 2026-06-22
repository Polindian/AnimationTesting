// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "CharacterEntryWidget.generated.h"

class UPA_CharacterDefinition;


/**
 * 
 */
UCLASS()
class UCharacterEntryWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
public:
	FORCEINLINE const UPA_CharacterDefinition* GetCharacterDefinition() const { return CharacterDefinition; }

	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	void SetSelected(bool bIsSelected);


private:
	UPROPERTY(meta = (BindWidget))
	class UImage* CharacterIcon;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CharacterNameText;

	UPROPERTY(EditDefaultsOnly, Category = "Character")
	FName IconTextureMaterialParamName = "Icon";

	UPROPERTY(EditDefaultsOnly, Category = "Character")
	FName SaturationMaterialParamName = "Saturation";

	UPROPERTY()
	const UPA_CharacterDefinition* CharacterDefinition;
	
};
