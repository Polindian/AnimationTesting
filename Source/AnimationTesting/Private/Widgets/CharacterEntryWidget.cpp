// Christopher Naglik All Rights Reserved


#include "Widgets/CharacterEntryWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Character/PA_CharacterDefinition.h"

void UCharacterEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
	CharacterDefinition = Cast<UPA_CharacterDefinition>(ListItemObject);

	if (CharacterDefinition)
	{
		CharacterIcon->GetDynamicMaterial()->SetTextureParameterValue(IconTextureMaterialParamName, CharacterDefinition->LoadIcon());
		CharacterNameText->SetText(FText::FromString(CharacterDefinition->GetCharacterDisplayName()));
	}
}

void UCharacterEntryWidget::SetSelected(bool bIsSelected)
{
	CharacterIcon->GetDynamicMaterial()->SetScalarParameterValue(SaturationMaterialParamName, bIsSelected ? 0 : 1);
}