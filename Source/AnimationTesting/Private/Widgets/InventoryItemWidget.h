// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Widgets/ItemWidget.h"
#include "InventoryItemWidget.generated.h"


class UInventoryItem;

/**
 * 
 */
UCLASS()
class UInventoryItemWidget : public UItemWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	bool IsEmpty() const;
	void SetSlotNumber(int NewSlotNumber);
	void UpdateInventoryItem(const UInventoryItem* Item);
	void EmptySlot();
	FORCEINLINE int GetSlotNumber() const { return SlotNumber; }
	
	void UpdateStackCount();

private:
	

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	UTexture2D* EmptyTexture;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* StackCountText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CooldownCountText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CooldownDurationText;

	UPROPERTY()
	const UInventoryItem* InventoryItem;

	int SlotNumber;

	// GAS

public:
	void StartCooldown(float CooldownDuration, float TimeRemaining);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	float CooldownUpdateInterval = 0.1f;

	void CooldownFinished();
	void UpdateCooldown();
	void ClearCooldown();

	FTimerHandle CooldownDurationTimerHandle;
	FTimerHandle CooldownUpdateTimerHandle;

	float CooldownTimeRemaining = 0.f;
	float CooldownTimeDuration = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	FName CooldownAmountDynamicMaterialParamName = "Percent";

	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	FName IconTextureDynamicParamName = "Icon";

	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	FName CanCastDynamicParamName = "CanCast";

	virtual void SetIcon(UTexture2D* IconTexture) override;
	FNumberFormattingOptions CooldownDisplayFormattingOptions;
};