// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "CharacterEntryWidget.generated.h"

class UPA_CharacterDefinition;
class UButton;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEntryHovered, const UPA_CharacterDefinition*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEntryClicked, const UPA_CharacterDefinition*);

/**
 * One hero tile. Focus drives the glow and tooltip (keyboard, gamepad, and
 * mouse via hover->focus); clicking/A actually picks the hero. Browsing and
 * picking are separate — moving focus never sends anything to the server.
 */
UCLASS()
class UCharacterEntryWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
public:
	FORCEINLINE const UPA_CharacterDefinition* GetCharacterDefinition() const { return CharacterDefinition; }

	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	void SetSelected(bool bIsSelected);

	FOnEntryHovered OnEntryHovered;   // focus arrived -> owner shows tooltip
	FOnEntryClicked OnEntryClicked;   // clicked/A -> owner sends the pick

	void FocusEntry();
	UButton* GetSelectButton() const { return SelectButton; }

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	UPROPERTY(meta = (BindWidget))
	class UImage* CharacterIcon;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CharacterNameText;

	// Fills the tile, transparent style — the click surface and nav target
	UPROPERTY(meta = (BindWidget))
	UButton* SelectButton;

	// Focus-driven highlight, same pattern as the team slots
	UPROPERTY(meta = (BindWidget))
	class UImage* HoverGlow;

	UPROPERTY(EditDefaultsOnly, Category = "Character")
	FName IconTextureMaterialParamName = "Icon";

	UPROPERTY(EditDefaultsOnly, Category = "Character")
	FName SaturationMaterialParamName = "Saturation";

	UPROPERTY()
	const UPA_CharacterDefinition* CharacterDefinition;

	UFUNCTION()
	void HandleClicked();
};