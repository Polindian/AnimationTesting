// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/MenuButtonWidget.h"
#include "GameplayMenu.generated.h"

class UGeneralMenuWidget;
class USettingsWidget;

UCLASS()
class UGameplayMenu : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeOnInitialized() override;

    // Return-to-arena is still driven by the controller's toggle
    FOnMenuButtonClicked& GetReturnToArenaButtonClickedEventDelegate();

    // Called by the controller each time the menu opens, so the first button
    // is focused for controller users
    void FocusDefaultButton();

    // Fired when the player confirms leaving — the controller handles the travel
    FSimpleMulticastDelegate OnLeaveMatchConfirmed;

private:
    UPROPERTY(meta = (BindWidget))
    class UMenuButtonWidget* ReturnToArenaButton;

    UPROPERTY(meta = (BindWidget))
    class UMenuButtonWidget* SettingsButton;

    UPROPERTY(meta = (BindWidget))
    class UMenuButtonWidget* LeaveMatchButton;

    // Overlays spawned on demand, reused afterwards
    UPROPERTY(EditDefaultsOnly, Category = "Menu")
    TSubclassOf<USettingsWidget> SettingsWidgetClass;

    UPROPERTY()
    USettingsWidget* SettingsWidgetInstance;

    UPROPERTY(EditDefaultsOnly, Category = "Menu")
    TSubclassOf<UGeneralMenuWidget> GeneralMenuClass;

    UPROPERTY()
    UGeneralMenuWidget* GeneralMenu;

    UFUNCTION()
    void LeaveMatchClicked();

    UFUNCTION()
    void SettingsClicked();

    void SettingsClosed();

    // True when this is the practice arena, which has no real match to lose
    bool IsPracticeArena() const;
};