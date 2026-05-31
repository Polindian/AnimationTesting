// Christopher Naglik All Rights Reserved

#include "Widgets/GameplayMenu.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/RetainerBox.h"

void UGameplayMenu::NativeConstruct()
{
    Super::NativeConstruct();

    LeaveMatchButton->OnClicked.AddDynamic(this, &UGameplayMenu::LeaveMatch);
    SettingsButton->OnClicked.AddDynamic(this, &UGameplayMenu::GoSettingsPage);

    ReturnToArenaButton->OnHovered.AddDynamic(this, &UGameplayMenu::OnReturnToArenaHovered);
    ReturnToArenaButton->OnUnhovered.AddDynamic(this, &UGameplayMenu::OnReturnToArenaUnhovered);

    SettingsButton->OnHovered.AddDynamic(this, &UGameplayMenu::OnSettingsHovered);
    SettingsButton->OnUnhovered.AddDynamic(this, &UGameplayMenu::OnSettingsUnhovered);

    LeaveMatchButton->OnHovered.AddDynamic(this, &UGameplayMenu::OnLeaveMatchHovered);
    LeaveMatchButton->OnUnhovered.AddDynamic(this, &UGameplayMenu::OnLeaveMatchUnhovered);
}

FOnButtonClickedEvent& UGameplayMenu::GetReturnToArenaButtonClickedEventDelegate()
{
    return ReturnToArenaButton->OnClicked;
}

void UGameplayMenu::SetRetainerHovered(URetainerBox* Retainer, bool bHovered)
{
    if (!Retainer) return;
    Retainer->SetEffectMaterial(bHovered ? HoverGradientMaterial : nullptr);
}

void UGameplayMenu::OnReturnToArenaHovered() { SetRetainerHovered(ReturnToArenaRetainer, true); }
void UGameplayMenu::OnReturnToArenaUnhovered() { SetRetainerHovered(ReturnToArenaRetainer, false); }
void UGameplayMenu::OnSettingsHovered() { SetRetainerHovered(SettingsRetainer, true); }
void UGameplayMenu::OnSettingsUnhovered() { SetRetainerHovered(SettingsRetainer, false); }
void UGameplayMenu::OnLeaveMatchHovered() { SetRetainerHovered(LeaveMatchRetainer, true); }
void UGameplayMenu::OnLeaveMatchUnhovered() { SetRetainerHovered(LeaveMatchRetainer, false); }

void UGameplayMenu::LeaveMatch()
{
    UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, true);
}

void UGameplayMenu::GoSettingsPage()
{
}