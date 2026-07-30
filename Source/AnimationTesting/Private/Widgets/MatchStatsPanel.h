// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/MatchStatsTypes.h"
#include "MatchStatsPanel.generated.h"

/**
 * One stats panel — used twice on the match stats screen: the gold instance
 * shows the match MVP (the same for every client) and the silver instance
 * shows the local player. Purely a display; it never reads state itself,
 * the owning widget hands it a finished stats entry.
 */
UCLASS()
class UMatchStatsPanel : public UUserWidget
{
    GENERATED_BODY()

public:
    // PlayerCount is the denominator in every "(2/6)" — the number of players in the match, so it's passed in rather than stored per stat
    void SetStats(const FPlayerMatchStats& Stats, int32 PlayerCount);

private:
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* PlayerNameText;

    // Each stat has a value and its rank among all players
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* KillsValueText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* KillsRankText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* KDValueText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* KDRankText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* CaptureValueText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* CaptureRankText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* DamageValueText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* DamageRankText;

    // "POSITION - 1/6" — the overall standing, ranked on experience
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* OverallRankText;

    // XP box: a plain progress bar fed by snapshotted values, NOT an attribute-bound gauge — a client can't read another player's attributes,
    // so the MVP's level could never be drawn from live data
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* LevelText;

    UPROPERTY(meta = (BindWidget))
    class UProgressBar* LevelProgressBar;
};