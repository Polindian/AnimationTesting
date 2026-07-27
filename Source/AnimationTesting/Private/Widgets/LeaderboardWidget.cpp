// Christopher Naglik All Rights Reserved

#include "Widgets/LeaderboardWidget.h"
#include "Widgets/LeaderboardEntryWidget.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Animation/WidgetAnimation.h"

void ULeaderboardWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetIsFocusable(true);

	// Bound once per lifetime — the stacking-duplicates rule
	FWidgetAnimationDynamicEvent CloseFinished;
	CloseFinished.BindDynamic(this, &ULeaderboardWidget::HandleCloseAnimFinished);
	BindToAnimationFinished(Anim_Close, CloseFinished);

	if (LeaderboardScrollBox)
	{
		LeaderboardScrollBox->OnUserScrolled.AddDynamic(this, &ULeaderboardWidget::HandleScrolled);
	}
}

void ULeaderboardWidget::OpenLeaderboard()
{
	bClosing = false;

	if (bDebugFill)
	{
		PopulateDebugEntries();   // fills AllEntries, sorted
	}
	RebuildFromScratch();

	PlayAnimation(Anim_Open);

	GetWorld()->GetTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				for (UWidget* Child : LeaderboardScrollBox->GetAllChildren())
				{
					if (ULeaderboardEntryWidget* First = Cast<ULeaderboardEntryWidget>(Child))
					{
						First->FocusEntry();
						break;
					}
				}
			}));
}

void ULeaderboardWidget::SetLeaderboardData(const TArray<FLeaderboardEntry>& InEntries)
{
	AllEntries = InEntries;
	AllEntries.Sort(FLeaderboardEntry::SortLeaderboard);  // guarantee order regardless of source
	RebuildFromScratch();
}

void ULeaderboardWidget::RebuildFromScratch()
{
	LeaderboardScrollBox->ClearChildren();
	NextIndexToLoad = 0;
	AppendPage();   // first 20
}

// Spawns the next PageSize rows starting at NextIndexToLoad
void ULeaderboardWidget::AppendPage()
{
	const int32 End = FMath::Min(NextIndexToLoad + PageSize, AllEntries.Num());

	for (int32 i = NextIndexToLoad; i < End; ++i)
	{
		ULeaderboardEntryWidget* Row = CreateWidget<ULeaderboardEntryWidget>(GetOwningPlayer(), EntryWidgetClass);
		if (!Row) continue;

		Row->InitializeEntry(i + 1, AllEntries[i], /*bIsLocalPlayer*/ false);
		Row->OnRowFocused.AddUObject(this, &ULeaderboardWidget::HandleRowFocused);
		LeaderboardScrollBox->AddChild(Row);

		if (UScrollBoxSlot* BoxSlot = Cast<UScrollBoxSlot>(Row->Slot))
		{
			BoxSlot->SetPadding(EntryPadding);
		}
	}

	NextIndexToLoad = End;
}

// Load the next 20 when the user scrolls near the bottom
void ULeaderboardWidget::HandleScrolled(float CurrentOffset)
{
	if (NextIndexToLoad >= AllEntries.Num()) return;   // everything already loaded

	const float Threshold = LeaderboardScrollBox->GetScrollOffsetOfEnd() - 200.f;   // 200px early
	if (CurrentOffset >= Threshold)
	{
		AppendPage();
	}
}

FReply ULeaderboardWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (Key == EKeys::BackSpace || Key == EKeys::Gamepad_FaceButton_Right)
	{
		if (!bClosing)
		{
			bClosing = true;
			PlayAnimation(Anim_Close);
		}
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void ULeaderboardWidget::HandleCloseAnimFinished()
{
	RemoveFromParent();
	OnLeaderboardClosed.Broadcast();
}

void ULeaderboardWidget::HandleRowFocused(int32 RowIndex)
{
	if (RowIndex >= NextIndexToLoad - 3)
	{
		AppendPage();
	}
}

// 20 fake players, pre-varied so the sort has something to do
void ULeaderboardWidget::PopulateDebugEntries()
{
	AllEntries.Empty();
	for (int32 i = 0; i < 20; ++i)
	{
		FLeaderboardEntry E;
		E.PlayerName = FString::Printf(TEXT("PLAYER %02d"), i + 1);
		E.Wins = 100 - i * 4;              // descending so ranks are obvious
		E.Losses = 20 + i * 2;
		E.Kills = 500 - i * 15;
		E.Deaths = 200 + i * 10;
		AllEntries.Add(E);
	}

	AllEntries.Sort(FLeaderboardEntry::SortLeaderboard);  // exercise the real comparator
}