// Christopher Naglik All Rights Reserved

#include "Widgets/MenuButtonWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/RetainerBox.h"
#include "Components/SizeBox.h"
#include "Audio/ChrisAudioSubsystem.h"

void UMenuButtonWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Once per LIFETIME, not per add-to-tree — NativeConstruct re-runs every time
    // an overlay containing this button is re-added to the viewport, and
    // AddDynamic stacks duplicates
    MainButton->OnClicked.AddDynamic(this, &UMenuButtonWidget::HandleClicked);
    MainButton->OnHovered.AddDynamic(this, &UMenuButtonWidget::HandleHovered);
    MainButton->OnUnhovered.AddDynamic(this, &UMenuButtonWidget::HandleUnhovered);

    MainButton->IsFocusable = true;
}

// Runs in the editor too — makes the label show up live in the Designer
void UMenuButtonWidget::SynchronizeProperties()
{
    Super::SynchronizeProperties();

    if (ButtonText)
    {
        ButtonText->SetText(ButtonLabel);
        if (ButtonFont.FontObject)
        {
            ButtonText->SetFont(ButtonFont);
        }
    }

    // Apply per-instance size to the internal SizeBox — this is why external
    // resizing "didn't work": the override always wins, so we drive the override
    if (RootSizeBox)
    {
        RootSizeBox->SetWidthOverride(ButtonWidth);
        RootSizeBox->SetHeightOverride(ButtonHeight);
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

void UMenuButtonWidget::SetButtonSize(float InWidth, float InHeight)
{
    ButtonWidth = InWidth;
    ButtonHeight = InHeight;

    // Same override SynchronizeProperties applies — without this the SizeBox wins
    if (RootSizeBox)
    {
        RootSizeBox->SetWidthOverride(ButtonWidth);
        RootSizeBox->SetHeightOverride(ButtonHeight);
    }
}

void UMenuButtonWidget::HandleClicked()
{
    // Play before broadcasting — a listener may switch pages and destroy this widget
    if (ClickedSoundTag.IsValid())
    {
        if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
        {
            Audio->Play2D(ClickedSoundTag);
        }
    }

    OnMenuButtonClicked.Broadcast();
    OnMenuButtonClickedWithLabel.Broadcast(ButtonLabel);
}

void UMenuButtonWidget::HandleHovered()
{
    FocusButton(true);
}

void UMenuButtonWidget::HandleUnhovered()
{
}

void UMenuButtonWidget::FocusButton(bool bPlaySound)
{
    if(!MainButton) { return; }

    bSuppressFocusSound = !bPlaySound;
    MainButton->SetFocus();

    // If this button already had focus, SetFocus fires no event and the flag would otherwise sit true and eat the next real hover
    bSuppressFocusSound = false;
}

void UMenuButtonWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
    Super::NativeOnAddedToFocusPath(InFocusEvent);

    if (bSuppressFocusSound)
    {
        bSuppressFocusSound = false;
    }
    else if (HoveredSoundTag.IsValid())
    {
        if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
        {
            Audio->Play2D(HoveredSoundTag);
        }
    }

    if (ButtonRetainer)
    {
        ButtonRetainer->SetEffectMaterial(HoverGradientMaterial);
    }
}

void UMenuButtonWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
    Super::NativeOnRemovedFromFocusPath(InFocusEvent);

    if (!ButtonRetainer) return;

    if (ButtonRetainer)
    {
        ButtonRetainer->SetEffectMaterial(nullptr);
    }
}


