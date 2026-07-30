// Christopher Naglik All Rights Reserved

#include "Widgets/MatchStatsPanel.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UMatchStatsPanel::SetStats(const FPlayerMatchStats& Stats, int32 PlayerCount)
{
    PlayerNameText->SetText(FText::FromString(Stats.PlayerName));

    // Raw values
    KillsValueText->SetText(FText::AsNumber(Stats.HeroKills));

    // K/D to two decimal places
    KDValueText->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), Stats.GetKD())));

    // Capture time to the nearest whole second
    CaptureValueText->SetText(FText::AsNumber(FMath::RoundToInt(Stats.CaptureSeconds)));

    // Damage to the nearest whole point
    DamageValueText->SetText(FText::AsNumber(FMath::RoundToInt(Stats.DamageInflicted)));

    // "(2/6)" — this player's position for that stat out of everyone in the match
    auto RankText = [PlayerCount](int32 Rank)
        {
            return FText::FromString(FString::Printf(TEXT("(%d/%d)"), Rank, PlayerCount));
        };

    KillsRankText->SetText(RankText(Stats.RankKills));
    KDRankText->SetText(RankText(Stats.RankKD));
    CaptureRankText->SetText(RankText(Stats.RankCapture));
    DamageRankText->SetText(RankText(Stats.RankDamage));

    OverallRankText->SetText(FText::FromString(
        FString::Printf(TEXT("POSITION - %d/%d"), Stats.RankOverall, PlayerCount)));

    // Level number, and how far through that level their XP sits
    LevelText->SetText(FText::AsNumber(FMath::FloorToInt(Stats.Level)));
    LevelProgressBar->SetPercent(Stats.GetLevelProgress());
}