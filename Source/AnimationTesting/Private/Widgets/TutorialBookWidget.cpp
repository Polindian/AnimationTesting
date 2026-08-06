// Christopher Naglik All Rights Reserved

#include "Widgets/TutorialBookWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Widgets/MenuButtonWidget.h"
#include "Audio/ChrisAudioSubsystem.h"
#include "Audio/ChrisGameplayTags.h"

void UTutorialBookWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Focus is required for NativeOnKeyDown to receive keyboard/gamepad input
	SetIsFocusable(true);

	Button_LeftArrow->OnClicked.AddDynamic(this, &UTutorialBookWidget::HandleLeftArrow);
	Button_RightArrow->OnClicked.AddDynamic(this, &UTutorialBookWidget::HandleRightArrow);

	// (ReturnButton binding deleted)

	FWidgetAnimationDynamicEvent Finished;
	Finished.BindDynamic(this, &UTutorialBookWidget::HandleBookAnimFinished);
	BindToAnimationFinished(Anim_OpenBook, Finished);
}

void UTutorialBookWidget::OpenBook()
{
	bClosing = false;

	if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
	{
		Audio->Play2D(ChrisGameplayTags::Audio_UI_Book_Open);
	}

	CurrentSpread = 0;
	RefreshSpread();
	PageContent->SetVisibility(ESlateVisibility::Hidden);
	PlayAnimation(Anim_OpenBook);

	// Deferred one tick: the widget was AddToViewport'd this same frame,
	// so its Slate widget doesn't exist yet and an immediate focus call fails
	GetWorld()->GetTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateWeakLambda(this, [this]() { SetKeyboardFocus(); }));
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

void UTutorialBookWidget::CloseBook()
{
	if (bClosing) { return; }
	bClosing = true;

	// Inside the guard, so spamming Esc gives one sound not five
	if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
	{
		Audio->Play2D(ChrisGameplayTags::Audio_UI_Book_Close);
	}

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

	if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
	{
		Audio->Play2D(ChrisGameplayTags::Audio_UI_Book_Flip);
	}

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
	if (Key == EKeys::BackSpace || Key == EKeys::Gamepad_FaceButton_Right)
	{
		CloseBook();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}