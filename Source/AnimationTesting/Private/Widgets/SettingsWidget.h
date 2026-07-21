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

    // Tab content roots inside TabSwitcher
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWidget> KeyboardTabRoot;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWidget> ControllerTabRoot;

    // Panel slides from off-screen right to center; also fades the blur in.
    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> Anim_SlideIn;

    // Panel slides from center to off-screen left; blur fades out
    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> Anim_SlideOut;

private:
    // Guards Backspace/B being spammed mid-close
    bool bClosing = false;

    UFUNCTION() void HandleKeyboardTab();
    UFUNCTION() void HandleControllerTab();
    UFUNCTION() void HandleSlideOutFinished();

    void CloseSettings();
};