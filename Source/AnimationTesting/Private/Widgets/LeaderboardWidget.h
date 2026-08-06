// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/LeaderboardTypes.h"
#include "LeaderboardWidget.generated.h"

class UWidgetAnimation;

UCLASS()
class ANIMATIONTESTING_API ULeaderboardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void OpenLeaderboard();
	FSimpleMulticastDelegate OnLeaderboardClosed;

	// The compatibility seam: hand it the full sorted dataset and it pages through it.
	// Real match-result data will call this exactly like the debug fill does.
	void SetLeaderboardData(const TArray<FLeaderboardEntry>& InEntries);

protected:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(meta = (BindWidget))
	class UScrollBox* LeaderboardScrollBox;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Anim_Open;    // top -> center, one bounce

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Anim_Close;   // slow drop, then whiz up off-screen

	UPROPERTY(EditDefaultsOnly, Category = "Leaderboard")
	TSubclassOf<class ULeaderboardEntryWidget> EntryWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Leaderboard")
	FMargin EntryPadding = FMargin(0.f, 4.f);

	// Fill 20 fake rows in PIE without a live backend
	UPROPERTY(EditDefaultsOnly, Category = "Leaderboard")
	bool bDebugFill = false;

private:
	// Whole sorted dataset; rows are spawned from it 20 at a time
	TArray<FLeaderboardEntry> AllEntries;
	int32 NextIndexToLoad = 0;

	static constexpr int32 PageSize = 20;

	bool bClosing = false;

	void AppendPage();
	void RebuildFromScratch();
	void PopulateDebugEntries();

	UFUNCTION()
	void HandleScrolled(float CurrentOffset);

	UFUNCTION()
	void HandleCloseAnimFinished();

	void HandleRowFocused(int32 RowIndex);

	void CloseLeaderboard();
};