// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerTeamSlotWidget.generated.h"

class UPA_CharacterDefinition;

/**
 * 
 */
UCLASS()
class UPlayerTeamSlotWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	void UpdateSlot(const FString& PlayerName, const UPA_CharacterDefinition* CharacterDefinition);

	// Resets the slot to empty state (no name, no icon)
	void ClearSlot();

	// Changes name text color to indicate locked-in state
	void SetLockedInVisual(bool bIsLockedIn);

private:
	UPROPERTY(meta=(BindWidget))
	class UImage* PlayerCharacterIcon;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* NameText;

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FName CharacterIconMaterialParamName = "Icon";

	// Colors for the player name text
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FSlateColor DefaultNameColor = FSlateColor(FLinearColor::White);

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FSlateColor LockedInNameColor = FSlateColor(FLinearColor::Green);

	FString CachedPlayerNameString;

	void UpdateNameText();
};
