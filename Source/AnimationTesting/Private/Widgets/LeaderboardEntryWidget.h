// Christopher Naglik All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/LeaderboardTypes.h"
#include "LeaderboardEntryWidget.generated.h"

UCLASS()
class ULeaderboardEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Rank is passed in (1-based) because the entry doesn't know its own list position
	void InitializeEntry(int32 Rank, const FLeaderboardEntry& Entry, bool bIsLocalPlayer);

	void FocusEntry();
	class UButton* GetRowButton() const { return RowButton; }

	// Tells the owner which row gained focus, so it can page in more rows near the bottom
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnRowFocused, int32 /*RowIndex*/);
	FOnRowFocused OnRowFocused;


protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;

private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* RankText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerNameText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* WinsText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* WinRateText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* KDText;

	// Background bar — tinted gold/silver/bronze/blue by rank
	UPROPERTY(meta = (BindWidget))
	class UImage* RowBackground;

	// Border drawn over the current player's row (starts hidden)
	UPROPERTY(meta = (BindWidget))
	class UImage* LocalPlayerHighlight;

	UPROPERTY(EditDefaultsOnly, Category = "Leaderboard")
	UTexture2D* GoldTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Leaderboard")
	UTexture2D* SilverTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Leaderboard")
	UTexture2D* BronzeTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Leaderboard")
	UTexture2D* StandardTexture;

	// Transparent button filling the row — the nav target, nothing happens on click
	UPROPERTY(meta = (BindWidget))
	class UButton* RowButton;

	UPROPERTY(meta = (BindWidget))
	class UImage* HoverGlow;

	int32 RowIndex = 0;   // set in InitializeEntry (Rank - 1)
};