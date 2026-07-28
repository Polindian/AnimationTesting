// Christopher Naglik All Rights Reserved


#include "Widgets/CharacterEntryWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Character/PA_CharacterDefinition.h"

void UCharacterEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// OnInitialized, not Construct — list entries get recycled and re-added
	SelectButton->OnClicked.AddDynamic(this, &UCharacterEntryWidget::HandleClicked);

	HoverGlow->SetVisibility(ESlateVisibility::Hidden);
}

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

void UCharacterEntryWidget::HandleClicked()
{
	OnEntryClicked.Broadcast(CharacterDefinition);
}

void UCharacterEntryWidget::FocusEntry()
{
	if (SelectButton)
	{
		SelectButton->SetFocus();
	}
}

void UCharacterEntryWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	// Route through focus — the focus path handler does glow + tooltip for all devices
	FocusEntry();
}

void UCharacterEntryWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);
	if (HoverGlow)
	{
		HoverGlow->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	OnEntryHovered.Broadcast(CharacterDefinition);
}

void UCharacterEntryWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);
	if (HoverGlow)
	{
		HoverGlow->SetVisibility(ESlateVisibility::Hidden);
	}
}

FReply UCharacterEntryWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Enter || Key == EKeys::Gamepad_FaceButton_Bottom || Key == EKeys::Virtual_Accept)
	{
		HandleClicked();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
