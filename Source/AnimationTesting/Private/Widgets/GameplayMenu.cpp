// Christopher Naglik All Rights Reserved

#include "Widgets/GameplayMenu.h"
#include "Widgets/GeneralMenuWidget.h"
#include "Widgets/SettingsWidget.h"
#include "Framework/ChrisGameMode.h"

void UGameplayMenu::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // OnInitialized, not Construct — this widget is reused across toggles and
    // AddDynamic would stack duplicate bindings each time
    LeaveMatchButton->OnMenuButtonClicked.AddDynamic(this, &UGameplayMenu::LeaveMatchClicked);
    SettingsButton->OnMenuButtonClicked.AddDynamic(this, &UGameplayMenu::SettingsClicked);
}

FOnMenuButtonClicked& UGameplayMenu::GetReturnToArenaButtonClickedEventDelegate()
{
    return ReturnToArenaButton->OnMenuButtonClicked;
}

void UGameplayMenu::FocusDefaultButton()
{
    // Deferred: the menu flips from Collapsed to Visible in the same frame,
    // and a collapsed widget has no geometry to focus
    GetWorld()->GetTimerManager().SetTimerForNextTick(
        FTimerDelegate::CreateWeakLambda(this, [this]()
            {
                if (ReturnToArenaButton) { ReturnToArenaButton->FocusButton(); }
            }));
}

// Practice runs as a standalone world, so the auth game mode is reachable here.
// In a real match on a client it returns null, which correctly reads as "not practice".
bool UGameplayMenu::IsPracticeArena() const
{
    if (const AChrisGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AChrisGameMode>() : nullptr)
    {
        return GM->IsPracticeMode();
    }
    return false;
}

void UGameplayMenu::LeaveMatchClicked()
{
    if (!GeneralMenuClass) { return; }

    if (!GeneralMenu)
    {
        GeneralMenu = CreateWidget<UGeneralMenuWidget>(GetOwningPlayer(), GeneralMenuClass);
    }
    if (!GeneralMenu) { return; }

    // No real match to forfeit in practice, so no loss warning
    const FText Message = IsPracticeArena()
        ? NSLOCTEXT("GameplayMenu", "LeavePractice", "ARE YOU SURE YOU WANT TO LEAVE THE PRACTICE ARENA?")
        : NSLOCTEXT("GameplayMenu", "LeaveMatch",
            "ARE YOU SURE YOU WANT TO LEAVE THE MATCH? THIS WILL BE RECORDED AS A LOSS IN YOUR PLAYER STATISTICS.");

    GeneralMenu->AddToViewport(210);   // above this menu (200)
    SetVisibility(ESlateVisibility::HitTestInvisible);

    GeneralMenu->OpenMenu(EGeneralMenuType::YesNo, Message)
        .AddWeakLambda(this, [this](bool bConfirmed)
            {
                SetVisibility(ESlateVisibility::Visible);

                if (bConfirmed)
                {
                    OnLeaveMatchConfirmed.Broadcast();
                }
                else
                {
                    // Cancelled — put focus back where they left off
                    LeaveMatchButton->FocusButton();
                }
            });
}

void UGameplayMenu::SettingsClicked()
{
    if (!SettingsWidgetClass) { return; }

    if (!SettingsWidgetInstance)
    {
        SettingsWidgetInstance = CreateWidget<USettingsWidget>(GetOwningPlayer(), SettingsWidgetClass);
        SettingsWidgetInstance->OnSettingsClosed.AddUObject(this, &UGameplayMenu::SettingsClosed);
    }

    if (!SettingsWidgetInstance->IsInViewport())
    {
        SettingsWidgetInstance->AddToViewport(210);
        SetVisibility(ESlateVisibility::HitTestInvisible);   // menu dead underneath
        SettingsWidgetInstance->OpenSettings();
    }
}

void UGameplayMenu::SettingsClosed()
{
    SetVisibility(ESlateVisibility::Visible);
    SettingsButton->FocusButton();
}