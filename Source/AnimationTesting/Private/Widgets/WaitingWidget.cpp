// Christopher Naglik All Rights Reserved


#include "Widgets/WaitingWidget.h"
#include "Components/TextBlock.h"

void UWaitingWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

// Wipe previous context's cancel behaviour, let the new caller bind its own
FOnButtonClickedEvent& UWaitingWidget::ClearAndGetButtonClickedEvent()
{
	CancelButton->OnClicked.Clear();
	return CancelButton->OnClicked;
}

void UWaitingWidget::SetWaitInfoText(const FText& WaitInfo, bool bAllowCancel)
{
	if (CancelButton)
	{
		CancelButton->SetVisibility(bAllowCancel ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

	if (WaitInfoText)
	{
		WaitInfoText->SetText(WaitInfo);
	}
}

void UWaitingWidget::FocusCancelButton()
{
	if (CancelButton) { CancelButton->SetFocus(); } 
}
