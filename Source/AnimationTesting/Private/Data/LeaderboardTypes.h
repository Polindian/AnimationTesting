// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "LeaderboardTypes.generated.h"

USTRUCT()
struct FLeaderboardEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FString PlayerName;

	UPROPERTY()
	int32 Wins = 0;

	// Kept as raw counts so win % and K/D are computed, never stored pre-rounded —
	// rounding is display-only, and sorting needs the true values
	UPROPERTY()
	int32 Losses = 0;

	UPROPERTY()
	int32 Kills = 0;

	UPROPERTY()
	int32 Deaths = 0;

	float GetWinRate() const
	{
		const int32 Games = Wins + Losses;
		return Games > 0 ? (float)Wins / Games : 0.f;
	}

	float GetKD() const
	{
		// Deaths of 0 would divide by zero — treat as if 1 death, standard for K/D display
		return Deaths > 0 ? (float)Kills / Deaths : (float)Kills;
	}

	// A sorts ABOVE B when this returns true
	static bool SortLeaderboard(const FLeaderboardEntry& A, const FLeaderboardEntry& B)
	{
		if (A.Wins != B.Wins)          return A.Wins > B.Wins;               // 1st: more wins
		if (A.GetWinRate() != B.GetWinRate()) return A.GetWinRate() > B.GetWinRate(); // 2nd: higher win %
		return A.GetKD() > B.GetKD();                                        // 3rd: higher K/D
	}
};