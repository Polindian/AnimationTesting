// Christopher Naglik All Rights Reserved

#include "Widgets/MenuButtonWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/RetainerBox.h"

void UMenuButtonWidget::NativeConstruct()
{
    Super::NativeConstruct();

    MainButton->OnClicked.AddDynamic(this, &UMenuButtonWidget::HandleClicked);
    MainButton->OnHovered.AddDynamic(this, &UMenuButtonWidget::HandleHovered);
    MainButton->OnUnhovered.AddDynamic(this, &UMenuButtonWidget::HandleUnhovered);
}

// Runs in the editor too — makes the label show up live in the Designer
void UMenuButtonWidget::SynchronizeProperties()
{
    Super::SynchronizeProperties();
    if (ButtonText)
    {
        ButtonText->SetText(ButtonLabel);
    }
}

void UMenuButtonWidget::SetButtonText(const FText& InText)
{
    ButtonLabel = InText;
    if (ButtonText)
    {
        ButtonText->SetText(ButtonLabel);
    }
}

void UMenuButtonWidget::HandleClicked()
{
    OnMenuButtonClicked.Broadcast();
}

void UMenuButtonWidget::HandleHovered()
{
    if (ButtonRetainer)
    {
        ButtonRetainer->SetEffectMaterial(HoverGradientMaterial);
    }
}

void UMenuButtonWidget::HandleUnhovered()
{
    if (ButtonRetainer)
    {
        ButtonRetainer->SetEffectMaterial(nullptr);
    }
}