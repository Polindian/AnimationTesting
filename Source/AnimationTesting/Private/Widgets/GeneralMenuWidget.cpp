// Christopher Naglik All Rights Reserved

#include "Widgets/GeneralMenuWidget.h"
#include "Widgets/MenuButtonWidget.h"
#include "Components/TextBlock.h"

void UGeneralMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetIsFocusable(true);

	YesButton->OnMenuButtonClicked.AddDynamic(this, &UGeneralMenuWidget::HandleYes);
	NoButton->OnMenuButtonClicked.AddDynamic(this, &UGeneralMenuWidget::HandleNo);
	ContinueButton->OnMenuButtonClicked.AddDynamic(this, &UGeneralMenuWidget::HandleContinue);
}

FOnGeneralMenuClosed& UGeneralMenuWidget::OpenMenu(EGeneralMenuType Type, const FText& Message)
{
	MenuType = Type;
	MessageText->SetText(Message);

	const bool bYesNo = (Type == EGeneralMenuType::YesNo);
	YesButton->SetVisibility(bYesNo ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	NoButton->SetVisibility(bYesNo ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	ContinueButton->SetVisibility(bYesNo ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);

	// Default focus: the safe option — No for questions, Continue for notices
	GetWorld()->GetTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateWeakLambda(this, [this, bYesNo]()
			{
				(bYesNo ? NoButton : ContinueButton)->FocusButton();
			}));

	OnMenuClosed.Clear();
	return OnMenuClosed;
}

FReply UGeneralMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	// B / Backspace = back out: No for questions, dismiss for notices
	if (Key == EKeys::BackSpace || Key == EKeys::Gamepad_FaceButton_Right)
	{
		CloseMenu(MenuType == EGeneralMenuType::Continue);
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UGeneralMenuWidget::HandleYes() { CloseMenu(true); }
void UGeneralMenuWidget::HandleNo() { CloseMenu(false); }
void UGeneralMenuWidget::HandleContinue() { CloseMenu(true); }

void UGeneralMenuWidget::CloseMenu(bool bConfirmed)
{
	RemoveFromParent();
	OnMenuClosed.Broadcast(bConfirmed);
}