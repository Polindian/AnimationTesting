// Christopher Naglik All Rights Reserved
#include "Widgets/BannerWidget.h"
#include "Components/Image.h"
#include "Animation/WidgetAnimation.h"
#include "Engine/Texture2D.h"

void UBannerWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Bound once per lifetime — BindToAnimationFinished stacks duplicates otherwise
	FWidgetAnimationDynamicEvent OpenFinished;
	OpenFinished.BindDynamic(this, &UBannerWidget::HandleOpenFinished);
	BindToAnimationFinished(Anim_Open, OpenFinished);

	FWidgetAnimationDynamicEvent CloseFinished;
	CloseFinished.BindDynamic(this, &UBannerWidget::HandleCloseFinished);
	BindToAnimationFinished(Anim_Close, CloseFinished);

	// Hidden until something is announced
	BannerRoot->SetVisibility(ESlateVisibility::Hidden);
}

void UBannerWidget::ShowBanner(EBannerType Type, int32 RoundNumber, uint8 WinningTeamId, uint8 LocalTeamId)
{
	if (UTexture2D* Texture = ResolveTexture(Type, RoundNumber, WinningTeamId, LocalTeamId))
	{
		BannerQueue.Add(Texture);

		// If nothing is on screen, start immediately; otherwise it waits its turn
		if (!bIsPlaying)
		{
			PlayNextInQueue();
		}
	}
}

// The whole point of sending type+team instead of a texture: each client picks
// its own art, so the same server call reads "WON" for one team and "LOST" for the other
UTexture2D* UBannerWidget::ResolveTexture(EBannerType Type, int32 RoundNumber, uint8 WinningTeamId, uint8 LocalTeamId) const
{
	// 255 = draw / no winner
	const bool bLocalPlayerWon = (WinningTeamId != 255) && (WinningTeamId == LocalTeamId);

	switch (Type)
	{
	case EBannerType::RoundNumber:
		// RoundNumber is 1-based, array is 0-based
		return RoundTextures.IsValidIndex(RoundNumber - 1) ? RoundTextures[RoundNumber - 1] : nullptr;

	case EBannerType::RoundResult:
		// 255 = draw, so nobody won or lost — same banner for both teams
		if (WinningTeamId == 255) { return RoundDrawTexture; }
		return bLocalPlayerWon ? RoundWonTexture : RoundLostTexture;

	case EBannerType::MatchResult:
		return bLocalPlayerWon ? MatchWonTexture : MatchLostTexture;

	case EBannerType::TeamTriumph:
		// Team 0 = Red, Team 1 = Blue — matches your flag system's TeamOne/TeamTwo
		return (WinningTeamId == 0) ? RedTeamTriumphTexture : BlueTeamTriumphTexture;
	}
	return nullptr;
}

void UBannerWidget::PlayNextInQueue()
{
	if (BannerQueue.Num() == 0)
	{
		bIsPlaying = false;
		BannerRoot->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	bIsPlaying = true;

	// Anim_Close ends at 0 opacity — restore before opening again
	BannerRoot->SetRenderOpacity(1.f);

	BannerContent->SetBrushFromTexture(BannerQueue[0], false);
	BannerQueue.RemoveAt(0);

	BannerRoot->SetVisibility(ESlateVisibility::HitTestInvisible);
	PlayAnimation(Anim_Open);
}

// Fully open — hold, then close
void UBannerWidget::HandleOpenFinished()
{
	GetWorld()->GetTimerManager().SetTimer(HoldTimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [this]() { PlayAnimation(Anim_Close); }),
		HoldDuration, false);
}

// Fully closed — next queued banner, or go idle
void UBannerWidget::HandleCloseFinished()
{
	PlayNextInQueue();
}