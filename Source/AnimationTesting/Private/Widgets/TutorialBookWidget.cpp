// Christopher Naglik All Rights Reserved

#include "Widgets/TutorialBookWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Widgets/MenuButtonWidget.h"

void UTutorialBookWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Focus is required for NativeOnKeyDown to receive keyboard/gamepad input
	SetIsFocusable(true);

	Button_LeftArrow->OnClicked.AddDynamic(this, &UTutorialBookWidget::HandleLeftArrow);
	Button_RightArrow->OnClicked.AddDynamic(this, &UTutorialBookWidget::HandleRightArrow);

	ReturnButton->OnMenuButtonClicked.AddDynamic(this, &UTutorialBookWidget::HandleReturnClicked);

	// Bound in OnInitialized (runs once per lifetime) instead of Construct —
	// BindToAnimationFinished stacks duplicates if bound repeatedly
	FWidgetAnimationDynamicEvent Finished;
	Finished.BindDynamic(this, &UTutorialBookWidget::HandleBookAnimFinished);
	BindToAnimationFinished(Anim_OpenBook, Finished);
}

void UTutorialBookWidget::OpenBook()
{
	bClosing = false;
	CurrentSpread = 0;
	RefreshSpread();

	// Pages stay hidden until the book finishes growing
	PageContent->SetVisibility(ESlateVisibility::Hidden);

	// Steal focus so arrow keys / LB / RB / back keys route to this widget
	SetKeyboardFocus();
	PlayAnimation(Anim_OpenBook);
}

// Fires after BOTH the forward and the reversed play — bClosing decides which path we're on
void UTutorialBookWidget::HandleBookAnimFinished()
{
	if (bClosing)
	{
		RemoveFromParent();
		OnBookClosed.Broadcast();
	}
	else
	{
		// SelfHitTestInvisible: the panel ignores clicks but the arrow buttons inside still work
		PageContent->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}


void UTutorialBookWidget::HandleReturnClicked()
{
	// Same path as the back keys — CloseBook already guards against double-triggering
	CloseBook();
}


void UTutorialBookWidget::CloseBook()
{
	// Guard against Esc being spammed mid-close
	if (bClosing) { return; }
	bClosing = true;

	// Hide content first so the shrinking book is just the cover, mirroring the open
	PageContent->SetVisibility(ESlateVisibility::Hidden);
	PlayAnimationReverse(Anim_OpenBook);
}

void UTutorialBookWidget::HandleLeftArrow()
{
	ChangeSpread(-1);
	// Clicking can move keyboard focus — retake it so keys keep working
	SetKeyboardFocus();
}

void UTutorialBookWidget::HandleRightArrow()
{
	ChangeSpread(1);
	SetKeyboardFocus();
}

void UTutorialBookWidget::ChangeSpread(int32 Delta)
{
	// Clamp handles the edges — pressing right on 10/10 or left on 1/10 just does nothing
	const int32 NewSpread = FMath::Clamp(CurrentSpread + Delta, 0, NumSpreads() - 1);
	if (NewSpread == CurrentSpread) { return; }
	CurrentSpread = NewSpread;
	RefreshSpread();
}

// Single source of truth for what's on screen: page textures, counter text, arrow visibility
void UTutorialBookWidget::RefreshSpread()
{
	if (Pages.Num() < 2) { return; }

	Image_LeftPage->SetBrushFromTexture(Pages[CurrentSpread * 2]);
	Image_RightPage->SetBrushFromTexture(Pages[CurrentSpread * 2 + 1]);

	Text_PageCounter->SetText(FText::Format(
		NSLOCTEXT("TutorialBook", "PageCounter", "{0}/{1}"),
		FText::AsNumber(CurrentSpread + 1), FText::AsNumber(NumSpreads())));

	// Hidden (not Collapsed) keeps their layout slot so nothing shifts
	Button_LeftArrow->SetVisibility(CurrentSpread > 0
		? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	Button_RightArrow->SetVisibility(CurrentSpread < NumSpreads() - 1
		? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

FReply UTutorialBookWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (Key == EKeys::Right || Key == EKeys::Gamepad_RightShoulder)
	{
		ChangeSpread(1);
		return FReply::Handled();
	}
	if (Key == EKeys::Left || Key == EKeys::Gamepad_LeftShoulder)
	{
		ChangeSpread(-1);
		return FReply::Handled();
	}
	// Gamepad_FaceButton_Right = B on Xbox / Circle on PlayStation
	if (Key == EKeys::BackSpace || Key == EKeys::Gamepad_FaceButton_Right)
	{
		CloseBook();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}