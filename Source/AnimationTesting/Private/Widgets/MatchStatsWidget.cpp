// Christopher Naglik All Rights Reserved

#include "Widgets/MatchStatsWidget.h"
#include "Widgets/MatchStatsPanel.h"
#include "Widgets/MenuButtonWidget.h"
#include "Framework/ChrisGameState.h"
#include "GameFramework/PlayerState.h"
#include "Animation/WidgetAnimation.h"
#include "Audio/ChrisAudioSubsystem.h"
#include "Audio/ChrisGameplayTags.h"


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

void UMatchStatsWidget::FocusLeaveButton()
{
	if (!LeaveMatchButton || !LeaveMatchButton->GetIsEnabled()) { return; }

	// Deferred: focusing in the same frame as a visibility or enable change gets overridden
	GetWorld()->GetTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateWeakLambda(this, [this]() { LeaveMatchButton->FocusButton(); }));
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

void UMatchStatsWidget::NativeDestruct()
{
	if (StatsUpdatedHandle.IsValid())
	{
		if (AChrisGameState* GS = GetWorld() ? GetWorld()->GetGameState<AChrisGameState>() : nullptr)
		{
			GS->OnMatchStatsUpdated.Remove(StatsUpdatedHandle);
		}
		StatsUpdatedHandle.Reset();
	}

	Super::NativeDestruct();
}

void UMatchStatsWidget::ShowStats(APlayerState* LocalPlayerState)
{
	CachedLocalPlayerState = LocalPlayerState;

	AChrisGameState* GS = GetWorld() ? GetWorld()->GetGameState<AChrisGameState>() : nullptr;
	if (!GS) return;

	if (!StatsUpdatedHandle.IsValid())
	{
		StatsUpdatedHandle = GS->OnMatchStatsUpdated.AddUObject(
			this, &UMatchStatsWidget::HandleMatchStatsUpdated);
	}

	RefreshPanels();

	PlayAnimation(Anim_FadeIn);

	if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
	{
		Audio->Play2D(ChrisGameplayTags::Audio_UI_MatchStats);
	}
}

// Screen has fully arrived — now the button becomes usable and takes focus
void UMatchStatsWidget::HandleFadeInFinished()
{
	LeaveMatchButton->SetIsEnabled(true);
	FocusLeaveButton();

	UE_LOG(LogTemp, Warning, TEXT("[MatchStats] Fade finished — enabling button"));
}

void UMatchStatsWidget::HandleLeaveMatchClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("[MatchStats] Leave clicked"));
	OnLeaveMatchRequested.Broadcast();
}

void UMatchStatsWidget::RefreshPanels()
{
	AChrisGameState* GS = GetWorld() ? GetWorld()->GetGameState<AChrisGameState>() : nullptr;
	if (!GS) return;

	const TArray<FPlayerMatchStats>& AllStats = GS->GetMatchStats();
	const int32 PlayerCount = AllStats.Num();

	const FPlayerMatchStats* MVP = AllStats.FindByPredicate(
		[](const FPlayerMatchStats& S) { return S.RankOverall == 1; });

	const FPlayerMatchStats* Mine = AllStats.FindByPredicate(
		[this](const FPlayerMatchStats& S) { return S.OwningPlayer == CachedLocalPlayerState; });

	if (MVP && MVPPanel) { MVPPanel->SetStats(*MVP, PlayerCount); }
	if (Mine && PlayerPanel) { PlayerPanel->SetStats(*Mine, PlayerCount); }
}

void UMatchStatsWidget::HandleMatchStatsUpdated(const TArray<FPlayerMatchStats>& Stats)
{
	RefreshPanels();
}