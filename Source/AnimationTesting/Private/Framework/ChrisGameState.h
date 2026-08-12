// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Player/PlayerInfoTypes.h"
#include "Data/MatchStatsTypes.h"
#include "ChrisGameState.generated.h"

class UPA_CharacterDefinition;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerSelectionUpdated, const TArray<FPlayerSelection>& /*NewPlayerSelection*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMatchStatsUpdated, const TArray<FPlayerMatchStats>&);

/**
 * 
 */
UCLASS()
class AChrisGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	void RequestPlayerSelectionChange(const APlayerState* RequestingPlayer, uint8 DesiredSlot);
	bool IsSlotOccupied(uint8 SlotId) const;

	void SetCharacterSelected(const APlayerState* SelectingPlayer, const UPA_CharacterDefinition* SelectedDefinition);
	void SetCharacterDeselected(const APlayerState* RequestingPlayer);
	void RequestPlayerLockIn(const APlayerState* RequestingPlayer, bool bLockIn);

	FOnPlayerSelectionUpdated OnPlayerSelectionUpdated;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	const TArray<FPlayerSelection>& GetPlayerSelection() const;

	void RequestPlayerReadyChange(const APlayerState* RequestingPlayer, bool bReady);

	// Returns true if all players are readied AND teams are balanced
	bool CanStartHeroSelection() const;

	// Returns true if every player in the lobby is locked in
	bool CanStartMatch() const;

	// Called by the server when all players lock in or the timer expires.
	// Force-assigns random characters to anyone without a pick, then travels to the arena.
	void StartMatch();

	// Returns the remaining hero selection time (replicated so clients can display it)
	FORCEINLINE float GetHeroSelectionTimeRemaining() const { return HeroSelectionTimeRemaining; }

private:
	UPROPERTY(ReplicatedUsing = OnRep_PlayerSelectionArray)
	TArray<FPlayerSelection> PlayerSelectionArray;

	UFUNCTION()
	void OnRep_PlayerSelectionArray();

	UPROPERTY(Replicated)
	float HeroSelectionTimeRemaining;

	UPROPERTY(EditDefaultsOnly, Category = "Hero Selection")
	float HeroSelectionDuration = 60.f;

	// Timer handle for the countdown tick
	FTimerHandle HeroSelectionTimerHandle;

	// Called every second on the server to count down the hero selection timer
	void TickHeroSelectionTimer();

	// Starts the countdown timer — called once when all players transition to hero selection
	void StartHeroSelectionTimer();

	// Assigns a random character from the loaded definitions to any player who hasn't picked one
	void AssignRandomCharactersToEmptySlots();

	/************************/
	/*    Match Stats       */
	/************************/

public:
	// Tracking entry points — server only
	void AddHeroKill(AActor* Killer);
	void AddDeath(AActor* Victim);
	void AddCaptureTime(AActor* Player, float DeltaSeconds);
	void AddDamageDealt(AActor* Dealer, float Amount);

	// Called at match end: snapshots XP and fills every rank
	void FinalizeMatchStats();

	const TArray<FPlayerMatchStats>& GetMatchStats() const { return MatchStatsArray; }
	FOnMatchStatsUpdated OnMatchStatsUpdated;

private:
	UPROPERTY(ReplicatedUsing = OnRep_MatchStatsArray)
	TArray<FPlayerMatchStats> MatchStatsArray;

	UFUNCTION()
	void OnRep_MatchStatsArray();

	// Resolves an actor (pawn or controller) to its stats entry, creating one if needed.
	// Returns null for AI, which has no PlayerState — so AI never accrues stats.
	FPlayerMatchStats* FindOrAddStatsFor(AActor* Actor);

	void SubmitMatchResultsToLeaderboard();


/*************************/
/*   Arena Ambience      */
/*************************/

public:
	// Server only. Replicates to every client, which starts or stops the loop locally.
	void SetArenaAmbienceActive(bool bActive);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_ArenaAmbienceActive)
	bool bArenaAmbienceActive = false;

	UFUNCTION()
	void OnRep_ArenaAmbienceActive();

	// Applies the current state locally — called from OnRep on clients and
	// directly on the server, which never gets an OnRep of its own
	void ApplyArenaAmbienceState();

	UPROPERTY(Transient)
	UAudioComponent* ArenaAmbienceAudio = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	float AmbienceFadeInTime = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	float AmbienceFadeOutTime = 3.f;


};
