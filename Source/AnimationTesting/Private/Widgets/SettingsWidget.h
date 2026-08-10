// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsWidget.generated.h"

class UWidgetSwitcher;
class UWidgetAnimation;
class UBorder;

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

    // Fired after the slide-out finishes and the widget removes itself — the owner must refocus a button on its own page for controller users
    FSimpleMulticastDelegate OnSettingsClosed;

  
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

    // One border per tab — parents each tab button in the WBP. The border of the
    // ACTIVE tab stays filled; hovering any tab fills its border too.
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UBorder> KeyboardTabBorder;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UBorder> ControllerTabBorder;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UBorder> AVTabBorder;

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

    // Fill colour of the tab you are currently ON — highest alpha, most prominent
    UPROPERTY(EditDefaultsOnly, Category = "Settings|Tabs")
    FLinearColor TabActiveColor = FLinearColor(0.55f, 0.35f, 0.05f, 1.f);

    // Fill colour while hovering a NON-active tab — lower alpha than active
    UPROPERTY(EditDefaultsOnly, Category = "Settings|Tabs")
    FLinearColor TabHoveredColor = FLinearColor(0.55f, 0.35f, 0.05f, 0.5f);

    // Colour of inactive, unhovered tabs — fully transparent by default
    UPROPERTY(EditDefaultsOnly, Category = "Settings|Tabs")
    FLinearColor TabInactiveColor = FLinearColor(0, 0, 0, 0);

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UVolumeSliderWidget> Row_Master;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UVolumeSliderWidget> Row_Music;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UVolumeSliderWidget> Row_SFX;


/*****************************/
/*            Tabs           */
/*****************************/


private:
    // Guards Backspace/B being spammed mid-close
    bool bClosing = false;

    // Ordered lists so arrow keys / LB / RB can cycle by index.
    // TabBorders[i] pairs with TabOrder[i].
    TArray<UWidget*> TabOrder;
    TArray<UBorder*> TabBorders;

    // Which tab is currently active (index into TabOrder)
    int32 CurrentTabIndex = 0;

    // Moves to the next or previous tab, wrapping around at the edges
    void ChangeTab(int32 Delta);

    // Single source of truth: active tab filled, all others transparent
    void RefreshTabBorders();

    UFUNCTION() void HandleKeyboardTab();
    UFUNCTION() void HandleControllerTab();
    UFUNCTION() void HandleAVTab();
    UFUNCTION() void HandleSlideOutFinished();

    // Hover fills that tab's border; unhover restores via RefreshTabBorders,
    // so the ACTIVE tab keeps its fill after the mouse leaves
    UFUNCTION() void HandleKeyboardTabHovered();
    UFUNCTION() void HandleControllerTabHovered();
    UFUNCTION() void HandleAVTabHovered();
    UFUNCTION() void HandleTabUnhovered();

    void CloseSettings();

    // Switches the TabSwitcher to the tab at the given index and refreshes borders
    void SwitchToTab(int32 NewIndex, bool bPlaySound = true);


/*****************************/
/*            Audio          */
/*****************************/

    UFUNCTION() void HandleMasterVolumeChanged(float NewValue);
    UFUNCTION() void HandleMusicVolumeChanged(float NewValue);
    UFUNCTION() void HandleSFXVolumeChanged(float NewValue);

    // Pushes saved values into the sliders and refreshes the labels
    void RefreshVolumeSliders();

    void WireVolumeSliderNavigation();
};