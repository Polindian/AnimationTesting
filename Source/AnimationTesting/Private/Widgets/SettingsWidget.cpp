// Christopher Naglik All Rights Reserved

#include "Widgets/SettingsWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Audio/ChrisAudioSubsystem.h"
#include "Audio/ChrisGameplayTags.h"
#include "Widgets/MenuButtonWidget.h"
#include "Widgets/VolumeSliderWidget.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Audio/ChrisAudioSubsystem.h"
#include "Audio/ChrisGameUserSettings.h"
#include "Widgets/QualityOptionWidget.h"


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

    Row_Master->OnVolumeChanged.AddUObject(this, &USettingsWidget::HandleMasterVolumeChanged);
    Row_Music->OnVolumeChanged.AddUObject(this, &USettingsWidget::HandleMusicVolumeChanged);
    Row_SFX->OnVolumeChanged.AddUObject(this, &USettingsWidget::HandleSFXVolumeChanged);

    Option_Low->OnQualityOptionChosen.AddUObject(this, &USettingsWidget::HandleQualityChosen);
    Option_Medium->OnQualityOptionChosen.AddUObject(this, &USettingsWidget::HandleQualityChosen);
    Option_High->OnQualityOptionChosen.AddUObject(this, &USettingsWidget::HandleQualityChosen);

    WireVolumeSliderNavigation();
}

void USettingsWidget::OpenSettings()
{
    bClosing = false;
    SwitchToTab(0, false);
    RefreshVolumeSliders();
    RefreshQualityOptions();

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

    // Fires as the slide-out begins, not in HandleSlideOutFinished — by then
    // the panel is already off-screen
    if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
    {
        Audio->Play2D(ChrisGameplayTags::Audio_UI_Page_Leave);
    }

    if (UChrisGameUserSettings* Settings = UChrisGameUserSettings::GetChrisSettings())
    {
        Settings->ApplySettings(false);
    }

    PlayAnimation(Anim_SlideOut);
}

void USettingsWidget::SwitchToTab(int32 NewIndex, bool bPlaySound)
{
    const int32 PreviousIndex = CurrentTabIndex;
    CurrentTabIndex = FMath::Clamp(NewIndex, 0, TabOrder.Num() - 1);

    // Only sounds on an actual change — clicking the tab you're already on is silent
    if (bPlaySound && CurrentTabIndex != PreviousIndex)
    {
        if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
        {
            Audio->Play2D(ChrisGameplayTags::Audio_UI_Tab_Change);
        }
    }

    TabSwitcher->SetActiveWidget(TabOrder[CurrentTabIndex]);
    RefreshTabBorders();


    // AV tab is the only one with interactive controls, so it takes focus.
    // Deferred a tick — the switcher's new page has no Slate widget until it lays out.
    if (CurrentTabIndex == 2 && Row_Master)
    {
        GetWorld()->GetTimerManager().SetTimerForNextTick(
            FTimerDelegate::CreateWeakLambda(this, [this]()
                {
                    // Re-wired here, not in OnInitialized: the switcher hadn't built
                    // this page's Slate widgets yet, so the rules had nothing to bind to
                    WireVolumeSliderNavigation();
                    Row_Master->FocusSlider();
                }));
    }
}

void USettingsWidget::HandleMasterVolumeChanged(float NewValue)
{
    if (UChrisGameUserSettings* Settings = UChrisGameUserSettings::GetChrisSettings())
    {
        Settings->SetMasterVolume(NewValue);
    }

    if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
    {
        Audio->ApplyVolumeSettings();
    }

    PlayVolumeTick();
}

void USettingsWidget::HandleMusicVolumeChanged(float NewValue)
{
    if (UChrisGameUserSettings* Settings = UChrisGameUserSettings::GetChrisSettings())
    {
        Settings->SetMusicVolume(NewValue);
    }

    if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
    {
        Audio->ApplyVolumeSettings();
    }

    PlayVolumeTick();
}

void USettingsWidget::HandleSFXVolumeChanged(float NewValue)
{
    if (UChrisGameUserSettings* Settings = UChrisGameUserSettings::GetChrisSettings())
    {
        Settings->SetSFXVolume(NewValue);
    }

    if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
    {
        Audio->ApplyVolumeSettings();
    }

    PlayVolumeTick();
}

void USettingsWidget::RefreshVolumeSliders()
{
    const UChrisGameUserSettings* Settings = UChrisGameUserSettings::GetChrisSettings();
    if (!Settings) { return; }

    Row_Master->SetVolume(Settings->GetMasterVolume());
    Row_Music->SetVolume(Settings->GetMusicVolume());
    Row_SFX->SetVolume(Settings->GetSFXVolume());
}

void USettingsWidget::WireVolumeSliderNavigation()
{
    if (!Row_Master || !Row_Music || !Row_SFX) { return; }
    if (!Option_Low || !Option_Medium || !Option_High) { return; }

    Row_Master->SetNavigationRuleExplicit(EUINavigation::Down, Row_Music);
    Row_Master->SetNavigationRuleBase(EUINavigation::Up, EUINavigationRule::Stop);

    Row_Music->SetNavigationRuleExplicit(EUINavigation::Up, Row_Master);
    Row_Music->SetNavigationRuleExplicit(EUINavigation::Down, Row_SFX);

    Row_SFX->SetNavigationRuleExplicit(EUINavigation::Up, Row_Music);

    // Sliders eat Left and Right for nudging, so Down is the only way out —
    // without this the quality options are unreachable on a controller
    Row_SFX->SetNavigationRuleExplicit(EUINavigation::Down, Option_Low);

    Option_Low->SetNavigationRuleExplicit(EUINavigation::Up, Row_SFX);
    Option_Low->SetNavigationRuleExplicit(EUINavigation::Right, Option_Medium);

    // Left goes back to the sliders — Stop here would strand controller usersin the quality row with no way out
    Option_Low->SetNavigationRuleExplicit(EUINavigation::Left, Row_Master);

    Option_Medium->SetNavigationRuleExplicit(EUINavigation::Left, Option_Low);
    Option_Medium->SetNavigationRuleExplicit(EUINavigation::Right, Option_High);
    Option_Medium->SetNavigationRuleExplicit(EUINavigation::Up, Row_SFX);

    Option_High->SetNavigationRuleExplicit(EUINavigation::Left, Option_Medium);
    Option_High->SetNavigationRuleBase(EUINavigation::Right, EUINavigationRule::Stop);
    Option_High->SetNavigationRuleExplicit(EUINavigation::Up, Row_SFX);

    Row_Master->SetNavigationRuleExplicit(EUINavigation::Right, Option_Low);
    Row_Music->SetNavigationRuleExplicit(EUINavigation::Right, Option_Low);
    Row_SFX->SetNavigationRuleExplicit(EUINavigation::Right, Option_Low);

    Row_Master->BuildNavigation();
    Row_Music->BuildNavigation();
    Row_SFX->BuildNavigation();
    Option_Low->BuildNavigation();
    Option_Medium->BuildNavigation();
    Option_High->BuildNavigation();
}

void USettingsWidget::PlayVolumeTick()
{
    UWorld* World = GetWorld();
    if (!World) { return; }

    const double Now = World->GetTimeSeconds();
    if (Now - LastVolumeTickTime < VolumeTickInterval) { return; }

    LastVolumeTickTime = Now;

    if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
    {
        Audio->Play2D(ChrisGameplayTags::Audio_UI_Navigate_Soft);
    }
}

// Radio behaviour lives here, same as the tab borders — the rows only report
// being chosen, they never decide who's ticked
void USettingsWidget::RefreshQualityOptions()
{
    const UChrisGameUserSettings* Settings = UChrisGameUserSettings::GetChrisSettings();
    if (!Settings) { return; }

    const int32 Current = Settings->GetGraphicsQuality();

    Option_Low->SetChosen(Current == Option_Low->GetQualityLevel());
    Option_Medium->SetChosen(Current == Option_Medium->GetQualityLevel());
    Option_High->SetChosen(Current == Option_High->GetQualityLevel());
}

void USettingsWidget::HandleQualityChosen(int32 Level)
{
    if (UChrisGameUserSettings* Settings = UChrisGameUserSettings::GetChrisSettings())
    {
        Settings->SetGraphicsQuality(Level);
    }

    RefreshQualityOptions();

    if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
    {
        Audio->Play2D(ChrisGameplayTags::Audio_UI_Confirm);
    }

    UE_LOG(LogTemp, Warning, TEXT("[Graphics] Chose %d — engine reports %d, effects %d, shadows %d"),
        Level,
        UChrisGameUserSettings::GetChrisSettings()->GetOverallScalabilityLevel(),
        UChrisGameUserSettings::GetChrisSettings()->GetVisualEffectQuality(),
        UChrisGameUserSettings::GetChrisSettings()->GetShadowQuality());
}

void USettingsWidget::ChangeTab(int32 Delta)
{
    // Wrap around: going right past the last tab loops to the first, and vice versa
    int32 NewIndex = (CurrentTabIndex + Delta + TabOrder.Num()) % TabOrder.Num();
    SwitchToTab(NewIndex);

    // The AV tab hands focus to its first slider instead — taking it back here would fight that
    if (CurrentTabIndex != 2)
    {
        SetKeyboardFocus();
    }
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