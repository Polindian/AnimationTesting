// Christopher Naglik All Rights Reserved

#include "Widgets/SettingsWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Widgets/MenuButtonWidget.h"

void USettingsWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Focus is required for NativeOnKeyDown to receive keyboard/gamepad input
    SetIsFocusable(true);

    KeyboardTabButton->OnMenuButtonClicked.AddDynamic(this, &USettingsWidget::HandleKeyboardTab);
    ControllerTabButton->OnMenuButtonClicked.AddDynamic(this, &USettingsWidget::HandleControllerTab);

    // Bound in OnInitialized (runs once per lifetime) instead of Construct
    FWidgetAnimationDynamicEvent SlideOutFinished;
    SlideOutFinished.BindDynamic(this, &USettingsWidget::HandleSlideOutFinished);
    BindToAnimationFinished(Anim_SlideOut, SlideOutFinished);
}

void USettingsWidget::OpenSettings()
{
    bClosing = false;

    // Always land on the first tab when reopening
    TabSwitcher->SetActiveWidget(KeyboardTabRoot);

    // Steal focus so Backspace / gamepad B route to this widget
    SetKeyboardFocus();
    PlayAnimation(Anim_SlideIn);
}

void USettingsWidget::CloseSettings()
{
    // Guard against back keys being spammed mid-close
    if (bClosing) { return; }
    bClosing = true;

    PlayAnimation(Anim_SlideOut);
}

// Panel is fully off-screen left and the blur has faded — remove like the tutorial book does
void USettingsWidget::HandleSlideOutFinished()
{
    RemoveFromParent();
}

void USettingsWidget::HandleKeyboardTab()
{
    TabSwitcher->SetActiveWidget(KeyboardTabRoot);
    // Clicking can move keyboard focus — retake it so back keys keep working
    SetKeyboardFocus();
}

void USettingsWidget::HandleControllerTab()
{
    TabSwitcher->SetActiveWidget(ControllerTabRoot);
    SetKeyboardFocus();
}

FReply USettingsWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();

    // Gamepad_FaceButton_Right = B on Xbox / Circle on PlayStation
    if (Key == EKeys::BackSpace || Key == EKeys::Gamepad_FaceButton_Right)
    {
        CloseSettings();
        return FReply::Handled();
    }
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}