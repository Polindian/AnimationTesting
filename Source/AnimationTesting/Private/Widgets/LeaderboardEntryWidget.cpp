// Christopher Naglik All Rights Reserved
#include "Widgets/LeaderboardEntryWidget.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Components/Image.h"
#include "Components/Button.h"

void ULeaderboardEntryWidget::InitializeEntry(int32 Rank, const FLeaderboardEntry& Entry, bool bIsLocalPlayer)
{
	// Zero-padded rank: 01, 02, ... matches your mockup
	RankText->SetText(FText::FromString(FString::Printf(TEXT("%02d"), Rank)));
	PlayerNameText->SetText(FText::FromString(Entry.PlayerName));
	WinsText->SetText(FText::AsNumber(Entry.Wins));

	// Win %: nearest whole number
	const int32 WinPct = FMath::RoundToInt(Entry.GetWinRate() * 100.f);
	WinRateText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), WinPct)));

	// K/D: 2 decimal places
	KDText->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), Entry.GetKD())));

	UTexture2D* RowTexture = StandardTexture;
	if (Rank == 1) RowTexture = GoldTexture;
	else if (Rank == 2) RowTexture = SilverTexture;
	else if (Rank == 3) RowTexture = BronzeTexture;

	if (RowTexture)
	{
		RowBackground->SetBrushFromTexture(RowTexture);
	}

	LocalPlayerHighlight->SetVisibility(bIsLocalPlayer
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);

	RowIndex = Rank - 1;
	HoverGlow->SetVisibility(ESlateVisibility::Hidden);   // recycled rows never start glowing
}

void ULeaderboardEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	RowButton->IsFocusable = true;
	HoverGlow->SetVisibility(ESlateVisibility::Hidden);
}

void ULeaderboardEntryWidget::FocusEntry()
{
	if (RowButton) { RowButton->SetFocus(); }
}

// Mouse routes through focus so all three devices share one highlight path
void ULeaderboardEntryWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	FocusEntry();
}

void ULeaderboardEntryWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);
	HoverGlow->SetVisibility(ESlateVisibility::HitTestInvisible);
	OnRowFocused.Broadcast(RowIndex);
}

void ULeaderboardEntryWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);
	HoverGlow->SetVisibility(ESlateVisibility::Hidden);
}