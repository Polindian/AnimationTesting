// Christopher Naglik All Rights Reserved

#include "Widgets/SettingsWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Widgets/MenuButtonWidget.h"

void USettingsWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Focus is required for NativeOnKeyDown to receive keyboard/gamepad input
    SetIsFocusable(true);

    KeyboardTabButton->OnMenuButtonClicked.AddDynamic(this, &USettingsWidget::HandleKeyboardTab);
    ControllerTabButton->OnMenuButtonClicked.AddDynamic(this, &USettingsWidget::HandleControllerTab);
    AVTabButton->OnMenuButtonClicked.AddDynamic(this, &USettingsWidget::HandleAVTab);

    // Hover/unhover on the raw buttons inside each MenuButtonWidget — we drive
    // the tab borders from here, MenuButtonWidget knows nothing about tabs
    KeyboardTabButton->GetMainButton()->OnHovered.AddDynamic(this, &USettingsWidget::HandleKeyboardTabHovered);
    ControllerTabButton->GetMainButton()->OnHovered.AddDynamic(this, &USettingsWidget::HandleControllerTabHovered);
    AVTabButton->GetMainButton()->OnHovered.AddDynamic(this, &USettingsWidget::HandleAVTabHovered);

    KeyboardTabButton->GetMainButton()->OnUnhovered.AddDynamic(this, &USettingsWidget::HandleTabUnhovered);
    ControllerTabButton->GetMainButton()->OnUnhovered.AddDynamic(this, &USettingsWidget::HandleTabUnhovered);
    AVTabButton->GetMainButton()->OnUnhovered.AddDynamic(this, &USettingsWidget::HandleTabUnhovered);

    // Bound in OnInitialized (runs once per lifetime) instead of Construct
    FWidgetAnimationDynamicEvent SlideOutFinished;
    SlideOutFinished.BindDynamic(this, &USettingsWidget::HandleSlideOutFinished);
    BindToAnimationFinished(Anim_SlideOut, SlideOutFinished);

    // Define the tab cycling order — arrow keys and LB/RB step through these.
    // Borders are index-paired with the roots: TabBorders[i] belongs to TabOrder[i]
    TabOrder = { KeyboardTabRoot, ControllerTabRoot, AVTabRoot };
    TabBorders = { KeyboardTabBorder, ControllerTabBorder, AVTabBorder };
}

void USettingsWidget::OpenSettings()
{
    bClosing = false;
    SwitchToTab(0);
    PlayAnimation(Anim_SlideIn);

    // Deferred for the same first-open reason as the book
    GetWorld()->GetTimerManager().SetTimerForNextTick(
        FTimerDelegate::CreateWeakLambda(this, [this]() { SetKeyboardFocus(); }));
}

void USettingsWidget::CloseSettings()
{
    // Guard against back keys being spammed mid-close
    if (bClosing) { return; }
    bClosing = true;

    PlayAnimation(Anim_SlideOut);
}


void USettingsWidget::SwitchToTab(int32 NewIndex)
{
    CurrentTabIndex = FMath::Clamp(NewIndex, 0, TabOrder.Num() - 1);
    TabSwitcher->SetActiveWidget(TabOrder[CurrentTabIndex]);

    // Active tab gets the fill, everything else goes transparent
    RefreshTabBorders();
}

void USettingsWidget::ChangeTab(int32 Delta)
{
    // Wrap around: going right past the last tab loops to the first, and vice versa
    int32 NewIndex = (CurrentTabIndex + Delta + TabOrder.Num()) % TabOrder.Num();
    SwitchToTab(NewIndex);

    // Clicking can move keyboard focus — retake it so keys keep working
    SetKeyboardFocus();
}

// Single source of truth for tab border colours: the active tab is filled,
// all other tabs are transparent. Called on switch and on unhover.
void USettingsWidget::RefreshTabBorders()
{
    for (int32 i = 0; i < TabBorders.Num(); ++i)
    {
        if (TabBorders[i])
        {
            TabBorders[i]->SetBrushColor(i == CurrentTabIndex ? TabActiveColor : TabInactiveColor);
        }
    }
}

// Panel is fully off-screen left and the blur has faded — remove and tell the owner so it can refocus a button (controller users need something focused)
void USettingsWidget::HandleSlideOutFinished()
{
    RemoveFromParent();
    OnSettingsClosed.Broadcast();
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

// Hover fills that tab's border with the hover colour — but the ACTIVE tab
// keeps its stronger active colour even when hovered
void USettingsWidget::HandleKeyboardTabHovered()
{
    if (KeyboardTabBorder && CurrentTabIndex != 0)
    {
        KeyboardTabBorder->SetBrushColor(TabHoveredColor);
    }
}

void USettingsWidget::HandleControllerTabHovered()
{
    if (ControllerTabBorder && CurrentTabIndex != 1)
    {
        ControllerTabBorder->SetBrushColor(TabHoveredColor);
    }
}

void USettingsWidget::HandleAVTabHovered()
{
    if (AVTabBorder && CurrentTabIndex != 2)
    {
        AVTabBorder->SetBrushColor(TabHoveredColor);
    }
}
// Unhover: restore all borders from the single source of truth — the ACTIVE
// tab keeps its fill, non-active tabs return to transparent
void USettingsWidget::HandleTabUnhovered()
{
    RefreshTabBorders();
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