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
	TeamTriumph    // "RED TEAM TRIUMPH" / "BLUE TEAM TRIUMPH"
};

UCLASS()
class ANIMATIONTESTING_API UBannerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Queues a banner. Calling twice back-to-back (match result then triumph)
	// plays them in order rather than interrupting.
	void ShowBanner(EBannerType Type, int32 RoundNumber, uint8 WinningTeamId, uint8 LocalTeamId);

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

	// How long the banner sits fully open before closing
	UPROPERTY(EditDefaultsOnly, Category = "Banner")
	float HoldDuration = 3.f;

private:
	// Textures waiting to be shown, in order
	TArray<UTexture2D*> BannerQueue;
	bool bIsPlaying = false;

	FTimerHandle HoldTimerHandle;

	UTexture2D* ResolveTexture(EBannerType Type, int32 RoundNumber, uint8 WinningTeamId, uint8 LocalTeamId) const;
	void PlayNextInQueue();

	UFUNCTION() void HandleOpenFinished();
	UFUNCTION() void HandleCloseFinished();
};