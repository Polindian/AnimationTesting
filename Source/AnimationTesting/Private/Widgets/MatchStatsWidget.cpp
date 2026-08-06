// Christopher Naglik All Rights Reserved

#include "Widgets/MatchStatsWidget.h"
#include "Widgets/MatchStatsPanel.h"
#include "Widgets/MenuButtonWidget.h"
#include "Framework/ChrisGameState.h"
#include "GameFramework/PlayerState.h"
#include "Animation/WidgetAnimation.h"

void UMatchStatsWidget::PlayLeaveFade(float Duration)
{
	// Button off immediately so it can't be pressed twice mid-fade
	if (LeaveMatchButton) { LeaveMatchButton->SetIsEnabled(false); }

	// Simple opacity ramp on the content; the black background stays put, which means the screen is already black when travel fires
	const float Step = GetWorld()->GetDeltaSeconds() / FMath::Max(0.01f, Duration);
	GetWorld()->GetTimerManager().SetTimer(LeaveFadeTimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [this, Step]()
			{
				if (!ContentRoot) return;
				const float NewOpacity = FMath::Max(0.f, ContentRoot->GetRenderOpacity() - Step);
				ContentRoot->SetRenderOpacity(NewOpacity);
				if (NewOpacity <= 0.f)
				{
					GetWorld()->GetTimerManager().ClearTimer(LeaveFadeTimerHandle);
				}
			}), 0.016f, true);
}

void UMatchStatsWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetIsFocusable(false);   // the button is the focus target, not the root

	// Bound once per lifetime — BindToAnimationFinished stacks duplicates
	FWidgetAnimationDynamicEvent FadeFinished;
	FadeFinished.BindDynamic(this, &UMatchStatsWidget::HandleFadeInFinished);
	BindToAnimationFinished(Anim_FadeIn, FadeFinished);

	LeaveMatchButton->OnMenuButtonClicked.AddDynamic(this, &UMatchStatsWidget::HandleLeaveMatchClicked);
}

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

	PlayAnimation(Anim_FadeIn);
}

// Screen has fully arrived — now the button becomes usable and takes focus
void UMatchStatsWidget::HandleFadeInFinished()
{
	LeaveMatchButton->SetIsEnabled(true);

	UE_LOG(LogTemp, Warning, TEXT("[MatchStats] Fade finished — enabling button"));

	// Deferred: enabling and focusing in the same frame can be overridden
	GetWorld()->GetTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateWeakLambda(this, [this]() { LeaveMatchButton->FocusButton(); }));
}

void UMatchStatsWidget::HandleLeaveMatchClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("[MatchStats] Leave clicked"));
	OnLeaveMatchRequested.Broadcast();
}