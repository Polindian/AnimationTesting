// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/GameModeBase.h"
#include "GenericTeamAgentInterface.h"
#include "Framework/Flag.h"
#include "ChrisGameMode.generated.h"

// This enum represents every possible state of a match.
// The game flows through these in order, then loops from
// TransitionToArena back to Countdown for the next round.
UENUM(BlueprintType)
enum class EMatchPhase : uint8
{
    WaitingForPlayers,    // Waiting for enough players to join
    RoundIntro,           // Banner logic before countdown
    Countdown,            // 5,4,3,2,1,FIGHT! - input disabled
    InRound,              // Active gameplay - input enabled, timer running
    RoundEnd,             // "Round Over" - input disabled, 5s pause
    TransitionToShop,     // 3s fade to black -> swap to shop UI -> fade in
    ShopPhase,            // Shop open, 60s timer, continue button
    TransitionToArena,    // 3s fade to black -> swap to arena UI -> fade in
    MatchOver             // One team has won the match
};

UCLASS()
class AChrisGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
	AChrisGameMode();
    virtual void BeginPlay() override;

    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

    virtual APlayerController* SpawnPlayerController(ENetRole InRemoteRole, const FString& Options) override;
    virtual UClass* GetDefaultPawnClassForController_Implementation(AController* Controller) override;
    virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;
    virtual void HandleSeamlessTravelPlayer(AController*& C) override;

    // Called by the engine every time a player successfully joins, detects enough players to start game
    virtual void PostLogin(APlayerController* NewPlayer) override;

    // Called by PlayerController when a player clicks the Continue button.
    void OnPlayerVoteContinue(APlayerController* VotingPlayer);

    // === NEW === Call this from your death/kill handling code to track kills per round
    void ReportKill(FGenericTeamId KillerTeam);

    // Stop Purchasing logic 
    bool IsInShopPhase() const { return CurrentPhase == EMatchPhase::ShopPhase; }

private:
    FGenericTeamId GetTeamIDForPlayer(const AController* InController) const;
    AActor* FindNextStartSpotForTeam(const FGenericTeamId& TeamID);

    UPROPERTY(EditDefaultsOnly, Category = "Team")
    TSubclassOf<APawn> BackupPawn;

    // ---- Phase transition functions ----
    void StartCountdown();
    void CountdownTick();            // Runs every 1s during countdown
    void StartRound();               // Countdown -> InRound
    void EndRound();
    void StartTransitionToShop();    // Fade To Shop
    void OnTransitionToShopMidpoint(); // Swap widgets during black screen
    void StartShopPhase();           // Shop Timer
    void StartTransitionToArena();   // Fade To Arena
    void OnTransitionToArenaMidpoint(); // Swap Widgets during black screen


    void StartRoundIntro();

    UPROPERTY(EditDefaultsOnly, Category = "Match Timing")
    float BannerPhaseDuration = 7.f;   // must cover open + hold + close

    UPROPERTY(EditDefaultsOnly, Category = "Match Timing")
    float RoundResultBannerDelay = 3.f;

   
    void ShowRoundResultBanner();

    // Stashed because the banner fires on a timer, after EndRound has returned
    uint8 PendingBannerWinningTeamId = 255;
    int32 PendingBannerRound = 0;

    FTimerHandle BannerDelayTimerHandle;

    void ShowMatchResultBanner();

    // Stashed for the delayed match banner, same reason as the round one
    uint8 PendingMatchWinningTeamId = 0;

    UPROPERTY(EditDefaultsOnly, Category = "Match Timing")
    float MatchStatsDelay = 60.f;   // after MATCH result + TEAM TRIUMPH banners

    void ShowMatchStats();
    FTimerHandle MatchStatsTimerHandle;


    // Loop through player controllers in server
    void ForEachPlayerController(TFunctionRef<void(class AChrisPlayerController*)> Func);

    // State of match 
    EMatchPhase CurrentPhase = EMatchPhase::WaitingForPlayers;
    int32 CurrentRound = 0;
    int32 CountdownSecondsRemaining = 0;

    // === NEW === Track how many rounds each team has won
    int32 TeamOneRoundWins = 0;
    int32 TeamTwoRoundWins = 0;

    // === NEW === Per-round kill counters (reset each round)
    int32 TeamOneKills = 0;
    int32 TeamTwoKills = 0;

    // Track players voted 'Continue'
    TSet<APlayerController*> ContinueVoters;

    FTimerHandle PhaseTimerHandle; // timer for each phase
    FTimerHandle CountdownTickHandle; // pre-round countdown

    // Timing
    UPROPERTY(EditDefaultsOnly, Category = "Match Timing")
    float RoundDuration = 60.f;

    UPROPERTY(EditDefaultsOnly, Category = "Match Timing")
    float ShopDuration = 60.f;

    UPROPERTY(EditDefaultsOnly, Category = "Match Timing")
    float CountdownTime = 5.f;

    UPROPERTY(EditDefaultsOnly, Category = "Match Timing")
    float RoundEndWaitTime = 5.f;

    UPROPERTY(EditDefaultsOnly, Category = "Match Timing")
    float TransitionDuration = 3.f;

    UPROPERTY(EditDefaultsOnly, Category = "Match Timing")
    int32 ExpectedPlayerCount = 2;

    UPROPERTY(EditDefaultsOnly, Category = "Match Timing")
    float BlackScreenHoldDuration = 2.f;

    // === NEW === How many rounds a team needs to win the match
    UPROPERTY(EditDefaultsOnly, Category = "Match Rewards")
    int32 RoundsToWin = 2;

    // === NEW === Soul rewards given at the end of each round
    // Losers get more souls as a catch-up mechanic
    UPROPERTY(EditDefaultsOnly, Category = "Match Rewards")
    float WinnerSoulReward = 50.f;

    UPROPERTY(EditDefaultsOnly, Category = "Match Rewards")
    float LoserSoulReward = 100.f;

    // === NEW === Evaluates all match conditions to determine who won the round
    // Returns Neutral if the round should be restarted (all tiebreakers failed)
    EFlagOwnership DetermineRoundWinner();

    // === NEW === Awards souls based on round result, increments round wins
    EFlagOwnership AwardRoundEndSouls();

    // === NEW === Called when any flag is captured — checks if ALL flags are now captured
    void OnFlagCapturedCallback(EFlagOwnership CapturedByTeam);

    // === NEW === Called when a team reaches RoundsToWin — ends the match
    void EndMatch(bool bTeamOneWon);

    void OnTransitionToShopFadeIn();
    void OnTransitionToArenaFadeIn();

    UPROPERTY(EditDefaultsOnly, Category = "Team")
    TMap<FGenericTeamId, FName> TeamStartSpotTagMap;


	// --------- AI LOGIC ---------

    void StopAllAISpawning();
    void StartAllAISpawning();
    void DestroyAllAI();
    void TeleportPlayersToStart();

    void StopAllAIBehavior();

    bool IsPracticeMode() const;

    // Parsed once from the travel URL in InitGame
    bool bPracticeFromURL = false;

    UPROPERTY(EditDefaultsOnly, Category = "Practice Arena")
    int32 PracticeAIGroupSize = 5;

    // The team the player is on in practice mode — the OTHER team's barracks spawn
    UPROPERTY(EditDefaultsOnly, Category = "Practice Arena")
    uint8 PracticePlayerTeamId = 0;
};