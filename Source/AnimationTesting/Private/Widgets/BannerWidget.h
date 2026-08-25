// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BannerWidget.generated.h"

class UWidgetAnimation;

// What the banner is announcing. The client resolves the actual texture from
// this plus its own team, so one server call serves both teams correctly.
UENUM()
enum class EBannerType : uint8
{
	RoundNumber,   // "ROUND 1/2/3" — start of every round
	RoundResult,   // "ROUND WON" / "ROUND LOST"
	MatchResult,   // "MATCH WON" / "MATCH LOST"
	TeamTriumph,   // "RED TEAM TRIUMPH" / "BLUE TEAM TRIUMPH"
	PlayerDeath    // "YOU DIED" — local only, silent, lowest priority
};

// A banner waiting its turn. Stores the type as well as the art because the
// type still decides two things at play time: whether it stings, and whether
// match flow is allowed to shove it out of the way.
USTRUCT()
struct FBannerQueueEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UTexture2D> Texture = nullptr;

	UPROPERTY()
	EBannerType Type = EBannerType::RoundNumber;

	FBannerQueueEntry() {}
	FBannerQueueEntry(UTexture2D* InTexture, EBannerType InType)
		: Texture(InTexture), Type(InType) {
	}
};

UCLASS()
class ANIMATIONTESTING_API UBannerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Queues a banner. Calling twice back-to-back (match result then triumph)
	// plays them in order rather than interrupting.
	void ShowBanner(EBannerType Type, int32 RoundNumber, uint8 WinningTeamId, uint8 LocalTeamId);

	// Drops a death banner whether it's queued or on screen. Call on respawn so
	// a death banner can't outlive the death it belongs to.
	void ClearDeathBanner();

protected:
	virtual void NativeOnInitialized() override;

	// The text image that changes per banner — the scroll art itself is static in the WBP
	UPROPERTY(meta = (BindWidget))
	class UImage* BannerContent;

	// Root of the whole banner — scaled from the centre by the animations
	UPROPERTY(meta = (BindWidget))
	class UWidget* BannerRoot;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Anim_Open;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Anim_Close;

	// ROUND 1, ROUND 2, ROUND 3 — index 0 is round 1
	UPROPERTY(EditDefaultsOnly, Category = "Banner")
	TArray<UTexture2D*> RoundTextures;

	UPROPERTY(EditDefaultsOnly, Category = "Banner")
	UTexture2D* RoundWonTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Banner")
	UTexture2D* RoundLostTexture;

	// Shown when a round ends with no winner (all tiebreakers failed)
	UPROPERTY(EditDefaultsOnly, Category = "Banner")
	UTexture2D* RoundDrawTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Banner")
	UTexture2D* MatchWonTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Banner")
	UTexture2D* MatchLostTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Banner")
	UTexture2D* RedTeamTriumphTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Banner")
	UTexture2D* BlueTeamTriumphTexture;

	// "YOU DIED" — same scroll frame, no team variants
	UPROPERTY(EditDefaultsOnly, Category = "Banner")
	UTexture2D* YouDiedTexture;

	// How long the banner sits fully open before closing
	UPROPERTY(EditDefaultsOnly, Category = "Banner")
	float HoldDuration = 3.f;

	// YOU DIED sits longer — there's nothing else on screen worth reading
	// while you're dead, and it isn't holding match flow up
	UPROPERTY(EditDefaultsOnly, Category = "Banner")
	float DeathHoldDuration = 5.f;

private:
	// Banners waiting to be shown, in order
	TArray<FBannerQueueEntry> BannerQueue;
	bool bIsPlaying = false;

	// What's on screen right now — only meaningful while bIsPlaying
	EBannerType CurrentType = EBannerType::RoundNumber;

	// Set when a banner is being cut short. HandleOpenFinished reads it and
	// closes straight away instead of starting the hold timer, which covers
	// the case where we cut it short while Anim_Open is still running.
	bool bSkipHold = false;

	FTimerHandle HoldTimerHandle;

	UTexture2D* ResolveTexture(EBannerType Type, int32 RoundNumber, uint8 WinningTeamId, uint8 LocalTeamId) const;
	void PlayNextInQueue();
	void CutShortCurrentBanner();
	float GetHoldDurationForCurrentBanner() const;

	UFUNCTION() void HandleOpenFinished();
	UFUNCTION() void HandleCloseFinished();
};