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
    AVTabButton->OnMenuButtonClicked.AddDynamic(this, &USettingsWidget::HandleAVTab);

    // Bound in OnInitialized (runs once per lifetime) instead of Construct
    FWidgetAnimationDynamicEvent SlideOutFinished;
    SlideOutFinished.BindDynamic(this, &USettingsWidget::HandleSlideOutFinished);
    BindToAnimationFinished(Anim_SlideOut, SlideOutFinished);

    // Define the tab cycling order — arrow keys and LB/RB step through this array.
    TabOrder = { KeyboardTabRoot, ControllerTabRoot, AVTabRoot };
}

void USettingsWidget::OpenSettings()
{
    bClosing = false;

    // Always land on the first tab (Keyboard) when reopening
    SwitchToTab(0);


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

void USettingsWidget::SwitchToTab(int32 NewIndex)
{
    CurrentTabIndex = FMath::Clamp(NewIndex, 0, TabOrder.Num() - 1);
    TabSwitcher->SetActiveWidget(TabOrder[CurrentTabIndex]);
}

void USettingsWidget::ChangeTab(int32 Delta)
{
    // Wrap around: going right past the last tab loops to the first, and vice versa
    int32 NewIndex = (CurrentTabIndex + Delta + TabOrder.Num()) % TabOrder.Num();
    SwitchToTab(NewIndex);

    // Clicking can move keyboard focus — retake it so keys keep working
    SetKeyboardFocus();
}


void USettingsWidget::HandleKeyboardTab()
{
    SwitchToTab(0);
    SetKeyboardFocus();
}

void USettingsWidget::HandleControllerTab()
{
    SwitchToTab(1);
    SetKeyboardFocus();
}

void USettingsWidget::HandleAVTab()
{
    SwitchToTab(2);
    SetKeyboardFocus();
}


FReply USettingsWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();

    // Arrow keys (keyboard) cycle tabs left/right
    if (Key == EKeys::Right)
    {
        ChangeTab(1);
        return FReply::Handled();
    }
    if (Key == EKeys::Left)
    {
        ChangeTab(-1);
        return FReply::Handled();
    }

    // LB / RB (gamepad shoulder bumpers) cycle tabs
    if (Key == EKeys::Gamepad_RightShoulder)
    {
        ChangeTab(1);
        return FReply::Handled();
    }
    if (Key == EKeys::Gamepad_LeftShoulder)
    {
        ChangeTab(-1);
        return FReply::Handled();
    }

    // Gamepad_FaceButton_Right = B on Xbox / Circle on PlayStation
    if (Key == EKeys::BackSpace || Key == EKeys::Gamepad_FaceButton_Right)
    {
        CloseSettings();
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}