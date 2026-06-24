// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/PlayerInfoTypes.h"
#include "PlayerTeamLayoutWidget.generated.h"

class UPlayerTeamSlotWidget;

/**
 * 
 */
UCLASS()
class UPlayerTeamLayoutWidget : public UUserWidget
{
	GENERATED_BODY()
public:
    virtual void NativeConstruct() override;

    // Refreshes all slots with the latest player selection data
    void UpdatePlayerSelection(const TArray<FPlayerSelection>& PlayerSelections);

private:
    // Padding between each player slot in the horizontal box
    UPROPERTY(EditDefaultsOnly, Category = "Visual")
    float PlayerTeamWidgetSlotMargin = 5.f;

    // The widget class to spawn for each individual player slot
    UPROPERTY(EditDefaultsOnly, Category = "Visual")
    TSubclassOf<UPlayerTeamSlotWidget> PlayerTeamSlotWidgetClass;

    // Horizontal box for Red team slots (first half of indices)
    UPROPERTY(meta = (BindWidget))
    class UHorizontalBox* RedTeamLayoutBox;

    // Horizontal box for Blue team slots (second half of indices)
    UPROPERTY(meta = (BindWidget))
    class UHorizontalBox* BlueTeamLayoutBox;

    // All spawned slot widgets (indexed same as PlayerSelectionArray)
    UPROPERTY()
    TArray<UPlayerTeamSlotWidget*> TeamSlotWidgets;

};
