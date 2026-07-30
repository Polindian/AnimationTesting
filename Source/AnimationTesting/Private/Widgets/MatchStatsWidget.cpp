// Christopher Naglik All Rights Reserved


#include "Widgets/MatchStatsWidget.h"
#include "Widgets/MatchStatsPanel.h"
#include "Framework/ChrisGameState.h"
#include "GameFramework/PlayerState.h"

void UMatchStatsWidget::ShowStats(APlayerState* LocalPlayerState)
{
	AChrisGameState* GS = GetWorld() ? GetWorld()->GetGameState<AChrisGameState>() : nullptr;
	if (!GS) return;

	const TArray<FPlayerMatchStats>& AllStats = GS->GetMatchStats();
	const int32 PlayerCount = AllStats.Num();

	const FPlayerMatchStats* MVP = AllStats.FindByPredicate(
		[](const FPlayerMatchStats& S) { return S.RankOverall == 1; });

	const FPlayerMatchStats* Mine = AllStats.FindByPredicate(
		[LocalPlayerState](const FPlayerMatchStats& S) { return S.OwningPlayer == LocalPlayerState; });

	if (MVP && MVPPanel) { MVPPanel->SetStats(*MVP, PlayerCount); }
	if (Mine && PlayerPanel) { PlayerPanel->SetStats(*Mine, PlayerCount); }
}