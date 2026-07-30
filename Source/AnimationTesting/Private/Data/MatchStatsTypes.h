// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "MatchStatsTypes.generated.h"

USTRUCT()
struct FPlayerMatchStats
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<APlayerState> OwningPlayer = nullptr;

    UPROPERTY()
    FString PlayerName;

    UPROPERTY()
    int32 HeroKills = 0;
    UPROPERTY()
    int32 Deaths = 0;

    UPROPERTY()
    float CaptureSeconds = 0.f;

    UPROPERTY()
    float DamageInflicted = 0.f;

    // Snapshotted from CHeroAttributeSet at match end
    UPROPERTY()
    float Experience = 0.f;
    UPROPERTY()
    float Level = 1.f;
    UPROPERTY()
    float PrevLevelExperience = 0.f;
    UPROPERTY()
    float NextLevelExperience = 0.f;

    // 1-based ranks, filled at match end
    UPROPERTY()
    int32 RankKills = 0;
    UPROPERTY()
    int32 RankKD = 0;
    UPROPERTY()
    int32 RankCapture = 0;
    UPROPERTY()
    int32 RankDamage = 0;
    UPROPERTY()
    int32 RankOverall = 0;

    // 0 deaths -> treat as 1, same convention as the leaderboard
    float GetKD() const { return Deaths > 0 ? (float)HeroKills / Deaths : (float)HeroKills; }

    // Fraction through the current level, for the XP gauge
    float GetLevelProgress() const
    {
        const float Span = NextLevelExperience - PrevLevelExperience;
        return Span > 0.f ? FMath::Clamp((Experience - PrevLevelExperience) / Span, 0.f, 1.f) : 1.f;
    }
};