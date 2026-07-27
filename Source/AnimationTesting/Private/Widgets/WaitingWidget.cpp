// Christopher Naglik All Rights Reserved


#include "Widgets/WaitingWidget.h"
#include "Widgets/BackHintWidget.h"
#include "Components/TextBlock.h"

// Wipe previous context's cancel behaviour, let the new caller bind its own
FOnMenuButtonClicked& UWaitingWidget::ClearAndGetButtonClickedEvent()
{
	CancelButton->OnMenuButtonClicked.Clear();
	return CancelButton->OnMenuButtonClicked;
}

void UWaitingWidget::SetWaitInfoText(const FText& WaitInfo, bool bInAllowCancel)
{
	bAllowCancel = bInAllowCancel;

	if (CancelButton)
	{
		CancelButton->SetVisibility(bInAllowCancel ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

	// Hint only makes sense when B actually does something (CREATING LOBBY yes, LOGGING IN no)
	if (BackHint)
	{
		BackHint->SetVisibility(bInAllowCancel ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (WaitInfoText)
	{
		WaitInfoText->SetText(WaitInfo);
	}
}

void UWaitingWidget::FocusCancelButton()
{
	if (bAllowCancel && CancelButton)
	{
		CancelButton->FocusButton();
	}
}

// B routes through the same event the CANCEL button fires — one code path for both
void UWaitingWidget::TriggerCancel()
{
	if (bAllowCancel && CancelButton)
	{
		CancelButton->OnMenuButtonClicked.Broadcast();
	}
}