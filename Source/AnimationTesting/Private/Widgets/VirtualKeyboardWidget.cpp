// Christopher Naglik All Rights Reserved

#include "Widgets/VirtualKeyboardWidget.h"
#include "Widgets/MenuButtonWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Blueprint/WidgetTree.h"

void UVirtualKeyboardWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetIsFocusable(true);

	// Build the letter/number grid procedurally — 36 keys is too many to hand-place
	const TCHAR* Rows[] = { TEXT("1234567890"), TEXT("QWERTYUIOP"), TEXT("ASDFGHJKL"), TEXT("ZXCVBNM") };

	for (const TCHAR* Row : Rows)
	{
		UHorizontalBox* RowBox = WidgetTree->ConstructWidget<UHorizontalBox>();
		for (int32 i = 0; Row[i] != '\0'; ++i)
		{
			UMenuButtonWidget* Key = CreateWidget<UMenuButtonWidget>(this, KeyButtonClass);
			if (!Key) continue;

			Key->SetButtonText(FText::FromString(FString::Chr(Row[i])));
			Key->SetButtonSize(KeySize, KeySize);
			Key->OnMenuButtonClickedWithLabel.AddDynamic(this, &UVirtualKeyboardWidget::HandleKeyClicked);

			UHorizontalBoxSlot* KeySlot = RowBox->AddChildToHorizontalBox(Key);
			KeySlot->SetPadding(FMargin(4.f));

			if (Row[i] == 'G') { HomeKey = Key; }
		}
		KeyRowsBox->AddChildToVerticalBox(RowBox);
	}

	SpaceButton->OnMenuButtonClicked.AddDynamic(this, &UVirtualKeyboardWidget::HandleSpace);
	DeleteButton->OnMenuButtonClicked.AddDynamic(this, &UVirtualKeyboardWidget::HandleDelete);
	DoneButton->OnMenuButtonClicked.AddDynamic(this, &UVirtualKeyboardWidget::HandleDone);
}

void UVirtualKeyboardWidget::OpenKeyboard(const FText& InitialText)
{
	Buffer = InitialText.ToString();
	RefreshPreview();

	// Deferred: on first open, AddToViewport happened this same frame
	GetWorld()->GetTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				if (HomeKey) { HomeKey->FocusButton(); }
			}));
}

// Shortcuts bubble up from whichever key button is focused 
FReply UVirtualKeyboardWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (Key == EKeys::Gamepad_FaceButton_Right)  // B — cancel
	{
		CancelKeyboard();
		return FReply::Handled();
	}
	if (Key == EKeys::Gamepad_FaceButton_Left)   // X — delete
	{
		HandleDelete();
		return FReply::Handled();
	}
	if (Key == EKeys::Gamepad_FaceButton_Top)    // Y — space
	{
		HandleSpace();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UVirtualKeyboardWidget::HandleKeyClicked(const FText& Label)
{
	if (Buffer.Len() < MaxLength)
	{
		Buffer += Label.ToString();
		RefreshPreview();
	}
}

void UVirtualKeyboardWidget::HandleSpace()
{
	if (Buffer.Len() < MaxLength && !Buffer.IsEmpty())  // no leading spaces
	{
		Buffer += TEXT(" ");
		RefreshPreview();
	}
}

void UVirtualKeyboardWidget::HandleDelete()
{
	if (!Buffer.IsEmpty())
	{
		Buffer.LeftChopInline(1);
		RefreshPreview();
	}
}

void UVirtualKeyboardWidget::HandleDone()
{
	OnCommitted.Broadcast(FText::FromString(Buffer.TrimEnd()));
	RemoveFromParent();
}

void UVirtualKeyboardWidget::CancelKeyboard()
{
	OnCancelled.Broadcast();
	RemoveFromParent();
}

void UVirtualKeyboardWidget::RefreshPreview()
{
	// Trailing underscore doubles as a caret so an empty buffer still shows something
	PreviewText->SetText(FText::FromString(Buffer + TEXT("_")));
}