// Christopher Naglik All Rights Reserved


#include "Widgets/PlayerTeamSlotWidget.h"
#include "Character/PA_CharacterDefinition.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "PlayerTeamSlotWidget.h"

void UPlayerTeamSlotWidget::NativeConstruct()
{
    CachedPlayerNameString = "";
    PlayerCharacterIcon->SetVisibility(ESlateVisibility::Hidden);
}

void UPlayerTeamSlotWidget::UpdateSlot(const FString& PlayerName, const UPA_CharacterDefinition* CharacterDefinition)
{
    CachedPlayerNameString = PlayerName;
    UpdateNameText();

    if (CharacterDefinition)
    {
        // Player has picked a character — show the icon with their character's portrait
        PlayerCharacterIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        PlayerCharacterIcon->GetDynamicMaterial()->SetTextureParameterValue(CharacterIconMaterialParamName, CharacterDefinition->LoadIcon());
    }
    else
    {
        // No character selected yet — hide the icon, just show the name
        PlayerCharacterIcon->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UPlayerTeamSlotWidget::ClearSlot()
{
    CachedPlayerNameString = "";
    PlayerCharacterIcon->SetVisibility(ESlateVisibility::Hidden);
    UpdateNameText();
}

void UPlayerTeamSlotWidget::SetLockedInVisual(bool bIsLockedIn)
{
    // Turn player name green when locked in, white otherwise
    NameText->SetColorAndOpacity(bIsLockedIn ? LockedInNameColor : DefaultNameColor);
}


void UPlayerTeamSlotWidget::UpdateNameText()
{
    NameText->SetText(FText::FromString(CachedPlayerNameString));
}
