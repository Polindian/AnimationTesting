// Christopher Naglik All Rights Reserved


#include "Widgets/BannerWidget.h"
#include "Components/Image.h"
#include "Animation/WidgetAnimation.h"
#include "Engine/Texture2D.h"
#include "Audio/ChrisAudioSubsystem.h"
#include "Audio/ChrisGameplayTags.h"

namespace
{
	// Death is the one banner that arrives without ceremony — the death sound
	// is already playing on the owning client and the sting would fight it.
	bool ShouldPlaySting(EBannerType Type)
	{
		return Type != EBannerType::PlayerDeath;
	}

	// Match flow (rounds, match result, triumph) outranks a death banner.
	bool IsMatchFlowBanner(EBannerType Type)
	{
		return Type != EBannerType::PlayerDeath;
	}
}

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
	UTexture2D* Texture = ResolveTexture(Type, RoundNumber, WinningTeamId, LocalTeamId);
	if (!Texture)
	{
		return;
	}

	// Dying a moment before the round ends shouldn't hold ROUND WON behind YOU
	// DIED, so anything from match flow evicts a death banner rather than queueing
	if (IsMatchFlowBanner(Type))
	{
		BannerQueue.RemoveAll([](const FBannerQueueEntry& Entry)
			{
				return Entry.Type == EBannerType::PlayerDeath;
			});

		if (bIsPlaying && CurrentType == EBannerType::PlayerDeath)
		{
			CutShortCurrentBanner();
		}
	}

	BannerQueue.Add(FBannerQueueEntry(Texture, Type));

	// If nothing is on screen, start immediately; otherwise it waits its turn
	if (!bIsPlaying)
	{
		PlayNextInQueue();
	}
}

void UBannerWidget::ClearDeathBanner()
{
	BannerQueue.RemoveAll([](const FBannerQueueEntry& Entry)
		{
			return Entry.Type == EBannerType::PlayerDeath;
		});

	if (bIsPlaying && CurrentType == EBannerType::PlayerDeath)
	{
		CutShortCurrentBanner();
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

	case EBannerType::PlayerDeath:
		// Purely local — team and round mean nothing here
		return YouDiedTexture;
	}
	return nullptr;
}

void UBannerWidget::PlayNextInQueue()
{
	if (BannerQueue.Num() == 0)
	{
		bIsPlaying = false;
		bSkipHold = false;
		BannerRoot->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	bIsPlaying = true;
	bSkipHold = false;

	// Anim_Close ends at 0 opacity — restore before opening again
	BannerRoot->SetRenderOpacity(1.f);

	const FBannerQueueEntry Entry = BannerQueue[0];
	BannerQueue.RemoveAt(0);

	CurrentType = Entry.Type;
	BannerContent->SetBrushFromTexture(Entry.Texture, false);

	if (ShouldPlaySting(Entry.Type))
	{
		if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
		{
			Audio->Play2D(ChrisGameplayTags::Audio_UI_Banner);
		}
	}

	BannerRoot->SetVisibility(ESlateVisibility::HitTestInvisible);
	PlayAnimation(Anim_Open);
}

// Ends the on-screen banner early without hard-cutting it — it still closes
// through Anim_Close, so the next banner starts from a clean idle state
void UBannerWidget::CutShortCurrentBanner()
{
	if (IsAnimationPlaying(Anim_Close))
	{
		return;
	}

	// Still opening: let HandleOpenFinished close it rather than stopping the
	// animation mid-flight, which would leave the root at a partial scale
	if (IsAnimationPlaying(Anim_Open))
	{
		bSkipHold = true;
		return;
	}

	// Open and holding — drop the hold and close now
	GetWorld()->GetTimerManager().ClearTimer(HoldTimerHandle);
	PlayAnimation(Anim_Close);
}

float UBannerWidget::GetHoldDurationForCurrentBanner() const
{
	switch (CurrentType)
	{
	case EBannerType::PlayerDeath:
		return DeathHoldDuration;

	default:
		return HoldDuration;
	}
}

// Fully open — hold, then close
void UBannerWidget::HandleOpenFinished()
{
	// Evicted while we were still unfurling — skip the hold entirely
	if (bSkipHold)
	{
		bSkipHold = false;
		PlayAnimation(Anim_Close);
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(HoldTimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [this]() { PlayAnimation(Anim_Close); }),
		GetHoldDurationForCurrentBanner(), false);
}

// Fully closed — next queued banner, or go idle
void UBannerWidget::HandleCloseFinished()
{
	PlayNextInQueue();
}