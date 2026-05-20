// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GenericTeamAgentInterface.h"
#include "ChrisGameMode.generated.h"

// This enum represents every possible state of a match.
// The game flows through these in order, then loops from
// TransitionToArena back to Countdown for the next round.
UENUM(BlueprintType)
enum class EMatchPhase : uint8
{
    WaitingForPlayers,    // Waiting for enough players to join
    Countdown,            // 5,4,3,2,1,FIGHT! - input disabled
    InRound,              // Active gameplay - input enabled, timer running
    RoundEnd,             // "Round Over" - input disabled, 5s pause
    TransitionToShop,     // 3s fade to black ? swap to shop UI ? fade in
    ShopPhase,            // Shop open, 60s timer, continue button
    TransitionToArena     // 3s fade to black ? swap to arena UI ? fade in
};

UCLASS()
class AChrisGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    virtual APlayerController* SpawnPlayerController(ENetRole InRemoteRole, const FString& Options) override;

    // Called by the engine every time a player successfully joins, detects enough players to satrt game
    virtual void PostLogin(APlayerController* NewPlayer) override;

    // Called by PlayerController when a player clicks the Continue button.
    void OnPlayerVoteContinue(APlayerController* VotingPlayer);

private:
    FGenericTeamId GetTeamIDForPlayer(const APlayerController* PlayerController) const;
    AActor* FindNextStartSpotForTeam(const FGenericTeamId& TeamID);

    // ---- Phase transition functions ----
    void StartCountdown();           
    void CountdownTick();            // Runs every 1s during countdown
    void StartRound();               // Countdown InRound
    void EndRound();                 
    void StartTransitionToShop();    // Fade To Shop
    void OnTransitionToShopMidpoint(); // Swap widgets during black screen
    void StartShopPhase();           // Shop Timer
    void StartTransitionToArena();   // Fade To Arena
	void OnTransitionToArenaMidpoint(); // Swap Widgets during black screen

   
    // Loop through player controllers in server
    void ForEachPlayerController(TFunctionRef<void(class AChrisPlayerController*)> Func);

    // State of match 
    EMatchPhase CurrentPhase = EMatchPhase::WaitingForPlayers;
    int32 CurrentRound = 0;
    int32 CountdownSecondsRemaining = 0;

   
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

    void OnTransitionToShopFadeIn();
    void OnTransitionToArenaFadeIn();


    UPROPERTY(EditDefaultsOnly, Category = "Team")
    TMap<FGenericTeamId, FName> TeamStartSpotTagMap;

    void StopAllAISpawning();
    void StartAllAISpawning();
    void DestroyAllAI();
    void TeleportPlayersToStart();
};