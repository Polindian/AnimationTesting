// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsWidget.generated.h"

class UWidgetSwitcher;
class UWidgetAnimation;

/**
 * Full-screen settings overlay. Slides in from the right over a background blur,
 * Added to the viewport ON TOP of whatever screen is active
 */
UCLASS()
class ANIMATIONTESTING_API USettingsWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Resets to the first tab and plays the slide-in
    void OpenSettings();

protected:
    virtual void NativeOnInitialized() override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    // Switches between the tab content pages
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWidgetSwitcher> TabSwitcher;

    // Tab buttons across the top of the panel
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UMenuButtonWidget> KeyboardTabButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UMenuButtonWidget> ControllerTabButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UMenuButtonWidget> AVTabButton;

    // Tab content roots inside TabSwitcher
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWidget> KeyboardTabRoot;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWidget> ControllerTabRoot;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWidget> AVTabRoot;

    // Panel slides from off-screen right to center; also fades the blur in.
    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> Anim_SlideIn;

    // Panel slides from center to off-screen left; blur fades out
    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> Anim_SlideOut;

private:
    // Guards Backspace/B being spammed mid-close
    bool bClosing = false;

    // Ordered list of tab roots so arrow keys / LB / RB can cycle through them by index
    TArray<UWidget*> TabOrder;

    // Which tab is currently active (index into TabOrder)
    int32 CurrentTabIndex = 0;

    // Switches the TabSwitcher to the tab at the given index and retakes focus
    void SwitchToTab(int32 NewIndex);

    // Moves to the next or previous tab, wrapping around at the edges
    void ChangeTab(int32 Delta);

    UFUNCTION() void HandleKeyboardTab();
    UFUNCTION() void HandleControllerTab();
    UFUNCTION() void HandleAVTab();
    UFUNCTION() void HandleSlideOutFinished();

    void CloseSettings();
};