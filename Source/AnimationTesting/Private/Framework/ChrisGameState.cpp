// Christopher Naglik All Rights Reserved


#include "Framework/ChrisGameState.h"
#include "Player/LobbyPlayerController.h"
#include "Network/ChrisNetStatics.h"
#include "Framework/ChrisGameInstance.h"
#include "Framework/CAssetManager.h"
#include "Framework/ChrisGameMode.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h" 
#include "GAS/CHeroAttributeSet.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Audio/ChrisAudioSubsystem.h"
#include "Audio/ChrisGameplayTags.h"
#include "Components/AudioComponent.h"

void AChrisGameState::RequestPlayerSelectionChange(const APlayerState* RequestingPlayer, uint8 DesiredSlot)
{
	if(!HasAuthority() || IsSlotOccupied(DesiredSlot))
	{
		return;
	}

	// Check if this player already has an entry (they're switching slots, not picking for the first time)
	FPlayerSelection* PlayerSelectionPtr = PlayerSelectionArray.FindByPredicate([&] (const FPlayerSelection& PlayerSelection)
	{
		return PlayerSelection.IsForPlayer(RequestingPlayer);
		});

	if (PlayerSelectionPtr)
	{
		PlayerSelectionPtr->SetSlot(DesiredSlot);
	}
	else
	{
		PlayerSelectionArray.Add(FPlayerSelection(DesiredSlot, RequestingPlayer));
	}

	
	OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
}


// Assigns a character to the player. Duplicates allowed — multiple players can pick the same hero.
// Blocked if the player is already locked in.
void AChrisGameState::SetCharacterSelected(const APlayerState* SelectingPlayer, const UPA_CharacterDefinition* SelectedDefinition)
{
	if (!HasAuthority()) return;

	FPlayerSelection* FoundPlayerSelection = PlayerSelectionArray.FindByPredicate([&](const FPlayerSelection& PlayerSelected)
		{
			return PlayerSelected.IsForPlayer(SelectingPlayer);
		});

	// Can't change character if already locked in
	if (FoundPlayerSelection && !FoundPlayerSelection->GetIsLockedIn())
	{
		FoundPlayerSelection->SetCharacterDefinition(SelectedDefinition);
		
		OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
	}
}

bool AChrisGameState::IsSlotOccupied(uint8 SlotId) const
{
	for(const FPlayerSelection& PlayerSelection : PlayerSelectionArray)
	{
		if(PlayerSelection.GetPlayerSlot() == SlotId)
		{
			return true;
		}
	}

	return false;
}

void AChrisGameState::SetCharacterDeselected(const APlayerState* RequestingPlayer)
{
	if (!HasAuthority()) return;

	FPlayerSelection* FoundPlayerSelection = PlayerSelectionArray.FindByPredicate([&](const FPlayerSelection& PlayerSelection)
		{
			return PlayerSelection.IsForPlayer(RequestingPlayer);
		});

	if (FoundPlayerSelection && !FoundPlayerSelection->GetIsLockedIn())
	{
		FoundPlayerSelection->SetCharacterDefinition(nullptr);
		
		OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
	}
}

// Locks or unlocks the player's character selection (START MATCH / RETURN TO SELECTION)
void AChrisGameState::RequestPlayerLockIn(const APlayerState* RequestingPlayer, bool bLockIn)
{
	if (!HasAuthority()) return;

	FPlayerSelection* Found = PlayerSelectionArray.FindByPredicate([&](const FPlayerSelection& PS)
		{
			return PS.IsForPlayer(RequestingPlayer);
		});

	// Player must have a slot and a character selected before locking in
	if (Found && Found->GetCharacterDefinition())
	{
		Found->SetIsLockedIn(bLockIn);
		
		OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
	}

	// Check if all players are now locked in — if so, start the match immediately
	if (CanStartMatch())
	{
		StartMatch();
	}
}

// Registers PlayerSelectionArray for replication — fires OnRep on every change
void AChrisGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(AChrisGameState, PlayerSelectionArray, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(AChrisGameState, HeroSelectionTimeRemaining);
	DOREPLIFETIME_CONDITION_NOTIFY(AChrisGameState, MatchStatsArray, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(AChrisGameState, bArenaAmbienceActive);
}

const TArray<FPlayerSelection>& AChrisGameState::GetPlayerSelection() const
{
	return PlayerSelectionArray;
}

void AChrisGameState::RequestPlayerReadyChange(const APlayerState* RequestingPlayer, bool bReady)
{
	if (!HasAuthority()) return;

	FPlayerSelection* Found = PlayerSelectionArray.FindByPredicate([&](const FPlayerSelection& PlayerSelectionReady)
		{
			return PlayerSelectionReady.IsForPlayer(RequestingPlayer);
		});

	// Player must have a slot before they can ready up
	if (Found)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player tried to ready up!"));
		Found->SetIsReady(bReady);
		
		OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);

		// Auto-transition: if all players readied and teams balanced, switch everyone
		if (CanStartHeroSelection())	
		{
			StartHeroSelectionTimer();

			for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				ALobbyPlayerController* LobbyPC = Cast<ALobbyPlayerController>(It->Get());
				if (LobbyPC)
				{
					LobbyPC->Client_StartHeroSelection();
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Player tried to ready but has no slot!"));
	}
}

bool AChrisGameState::CanStartHeroSelection() const
{
	if (PlayerSelectionArray.Num() == 0) return false;

#if WITH_EDITOR
	// PIE shortcut: any number of readied players can proceed, teams ignored.
	// Compiled out of packaged builds entirely — this code won't exist in shipping.
	if (GIsEditor)
	{
		for (const FPlayerSelection& PS : PlayerSelectionArray)
		{
			if (!PS.GetIsReady()) return false;
		}
		return true;
	}
#endif
	int32 PlayersPerTeam = UChrisNetStatics::GetPlayerCountPerTeam();
	int32 RedCount = 0;
	int32 BlueCount = 0;

	for (const FPlayerSelection& PS : PlayerSelectionArray)
	{
		// Every player must be readied
		if (!PS.GetIsReady()) return false;

		// Count team membership
		if (PS.GetPlayerSlot() < PlayersPerTeam)
		{
			RedCount++;
		}
		else
		{
			BlueCount++;
		}
	}

	// Teams must have equal players
	return RedCount == BlueCount;
}




// Returns true when every player in the lobby has locked in their character
bool AChrisGameState::CanStartMatch() const
{
	if (PlayerSelectionArray.Num() == 0) return false;

	for (const FPlayerSelection& PS : PlayerSelectionArray)
	{
		if (!PS.GetIsLockedIn()) return false;
	}

	return true;
}

void AChrisGameState::StartMatch()
{
	if (!HasAuthority()) return;

	GetWorld()->GetTimerManager().ClearTimer(HeroSelectionTimerHandle);

	// Ensure every player has a character assigned
	AssignRandomCharactersToEmptySlots();

	// Travel to the game level via GameInstance
	UChrisGameInstance* GameInstance = GetGameInstance<UChrisGameInstance>();
	if (GameInstance)
	{
		GameInstance->StartMatch();
	}
}

void AChrisGameState::StartHeroSelectionTimer()
{
	if (!HasAuthority()) return;

	HeroSelectionTimeRemaining = HeroSelectionDuration;
	GetWorld()->GetTimerManager().SetTimer(HeroSelectionTimerHandle, this, &AChrisGameState::TickHeroSelectionTimer, 1.f, true);
}


// Called every second on the server and replicates to clients.When time runs out: force-lock everyone and start the match.
void AChrisGameState::TickHeroSelectionTimer()
{
	HeroSelectionTimeRemaining -= 1.f;
	

	if (HeroSelectionTimeRemaining <= 0.f)
	{
		HeroSelectionTimeRemaining = 0.f;

		// Force-lock every player who hasn't locked in yet
		for (FPlayerSelection& PS : PlayerSelectionArray)
		{
			if (!PS.GetIsLockedIn())
			{
				PS.SetIsLockedIn(true);
			}
		}

		OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
		StartMatch();
	}
}

// Goes through all player slots and assigns a random character to anyone who hasn't picked one.
// Uses the loaded character definitions from the asset manager.
void AChrisGameState::AssignRandomCharactersToEmptySlots()
{
	TArray<UPA_CharacterDefinition*> AllDefinitions;
	UCAssetManager::Get().GetLoadedCharacterDefinitions(AllDefinitions);

	if (AllDefinitions.Num() == 0) return;

	for (FPlayerSelection& PS : PlayerSelectionArray)
	{
		if (!PS.GetCharacterDefinition())
		{
			// Pick a random character from the loaded definitions
			int32 RandomIndex = FMath::RandRange(0, AllDefinitions.Num() - 1);
			PS.SetCharacterDefinition(AllDefinitions[RandomIndex]);
		}
	}

	
	OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
}

// Called automatically on clients when the server's PlayerSelectionArray replicates down
void AChrisGameState::OnRep_PlayerSelectionArray()
{
	OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
}

// Resolves any actor to its match-stats entry, creating one on first contact.
// Works from either a pawn or a controller because damage reports come from
// pawns while some kill reports come from controllers.
// Returns null for AI: skeletons have no PlayerState, so they can never
// accumulate stats of their own (though damage dealt TO them still counts
// for the player who dealt it).
FPlayerMatchStats* AChrisGameState::FindOrAddStatsFor(AActor* Actor)
{
	// A PlayerState is the only thing that survives respawns and round resets, so it's what identifies a player across the whole match
	APlayerState* PS = nullptr;
	if (APawn* Pawn = Cast<APawn>(Actor)) { PS = Pawn->GetPlayerState(); }
	else if (AController* C = Cast<AController>(Actor)) { PS = C->PlayerState; }

	// No PlayerState means AI or a world actor — nothing to track
	if (!PS) return nullptr;

	return FindOrAddStatsForPlayerState(PS);
}

FPlayerMatchStats* AChrisGameState::FindOrAddStatsForPlayerState(APlayerState* PS)
{
	if (!PS) return nullptr;

	// If we have already found an entry for this player, hand back a pointer to it
	FPlayerMatchStats* Found = MatchStatsArray.FindByPredicate(
		[PS](const FPlayerMatchStats& S) { return S.OwningPlayer == PS; });

	if (Found) return Found;

	FPlayerMatchStats NewStats;
	NewStats.OwningPlayer = PS;
	NewStats.PlayerName = PS->GetPlayerName();

	// EOS product user id — the key the leaderboard will store against
	if (PS->GetUniqueId().IsValid())
	{
		NewStats.UniquePlayerId = PS->GetUniqueId()->ToString();
	}

	return &MatchStatsArray[MatchStatsArray.Add(NewStats)];
}

void AChrisGameState::SeedStatsForAllPlayers()
{
	if (!HasAuthority()) return;

	for (APlayerState* PS : PlayerArray)
	{
		// Bots have no leaderboard identity, spectators didn't play
		if (!PS || PS->IsABot() || PS->IsOnlyASpectator()) { continue; }

		FindOrAddStatsForPlayerState(PS);
	}
}

void AChrisGameState::SubmitMatchResultsToLeaderboard()
{
	if (!HasAuthority()) return;

	// Practice is solo against AI — those results must never reach the leaderboard
	const AChrisGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AChrisGameMode>() : nullptr;
	if (GM && GM->IsPracticeMode())
	{
		UE_LOG(LogTemp, Log, TEXT("[Leaderboard] Practice match — results not submitted."));
		return;
	}

	for (const FPlayerMatchStats& S : MatchStatsArray)
	{
		if (!S.OwningPlayer) continue;

		// The leaderboard needs: a stable player id and result - Wins/losses come from the game mode's round tally, so that gets passed in when this is wired up.
		UE_LOG(LogTemp, Warning, TEXT("[Leaderboard] %s — Id:%s K:%d D:%d KD:%.2f"),
			*S.PlayerName, *S.UniquePlayerId, S.HeroKills, S.Deaths, S.GetKD());

		// TODO: POST to the Flask coordinator, or write to EOS stats
	}
}
void AChrisGameState::SetArenaAmbienceActive(bool bActive)
{
	if (!HasAuthority() || bArenaAmbienceActive == bActive) { return; }

	bArenaAmbienceActive = bActive;

	// OnRep doesn't fire on the server - listen server needs this
	ApplyArenaAmbienceState();
}

void AChrisGameState::OnRep_ArenaAmbienceActive()
{
	ApplyArenaAmbienceState();
}

void AChrisGameState::ApplyArenaAmbienceState()
{
	// Nobody is listening on a dedicated server
	if (GetNetMode() == NM_DedicatedServer) { return; }

	if (bArenaAmbienceActive)
	{
		if (!ArenaAmbienceAudio)
		{
			if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
			{
				ArenaAmbienceAudio = Audio->PlayLooping2DFadeIn(
					ChrisGameplayTags::Audio_Ambience_Arena, AmbienceFadeInTime);
			}
		}
	}
	else
	{
		UChrisAudioSubsystem::StopLoopingSound(ArenaAmbienceAudio, AmbienceFadeOutTime);
	}
}

void AChrisGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Travelling out mid-round would otherwise leave the loop running
	UChrisAudioSubsystem::StopLoopingSound(ArenaAmbienceAudio, 0.f);

	Super::EndPlay(EndPlayReason);
}

// Called from GAP_Dead when a hero kills another hero
void AChrisGameState::AddHeroKill(AActor* Killer)
{
	if (FPlayerMatchStats* S = FindOrAddStatsFor(Killer)) { S->HeroKills++; }
}

// Called from GAP_Dead for every hero death, including deaths with no killer (AI or environment)
void AChrisGameState::AddDeath(AActor* Victim)
{
	if (FPlayerMatchStats* S = FindOrAddStatsFor(Victim)) { S->Deaths++; }
}

// Called every tick from Flag for each hero standing in an uncaptured zone
void AChrisGameState::AddCaptureTime(AActor* Player, float DeltaSeconds)
{
	if (FPlayerMatchStats* S = FindOrAddStatsFor(Player)) { S->CaptureSeconds += DeltaSeconds; }
}

// Called from the attribute set whenever health is actually lost, with the true post-mitigation figure
void AChrisGameState::AddDamageDealt(AActor* Dealer, float Amount)
{
	if (FPlayerMatchStats* S = FindOrAddStatsFor(Dealer)) { S->DamageInflicted += Amount; }
}

// Runs once at match end. Two jobs: snapshots each player's XP state (which the
// stats screen can't read live, because a client can't query another player's
// attributes), then convert raw values into the 1-based positions the UI shows.
void AChrisGameState::FinalizeMatchStats()
{
	if (!HasAuthority()) return;

	SeedStatsForAllPlayers();

	// --- Snapshot XP/level from each player's pawn ---
	for (FPlayerMatchStats& S : MatchStatsArray)
	{
		if (!S.OwningPlayer) continue;

		// The pawn owns the ability system, so go PlayerState -> Pawn -> ASC
		APawn* Pawn = S.OwningPlayer->GetPawn();
		if (!Pawn) continue;

		IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Pawn);
		if (!ASI) continue;
		UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
		if (!ASC) continue;

		// Prev/Next level XP are stored so the gauge can show progress through the current level without recalculating from the experience curve
		bool bFound = false;
		S.Experience = ASC->GetGameplayAttributeValue(UCHeroAttributeSet::GetExperienceAttribute(), bFound);
		S.Level = ASC->GetGameplayAttributeValue(UCHeroAttributeSet::GetLevelAttribute(), bFound);
		S.PrevLevelExperience = ASC->GetGameplayAttributeValue(UCHeroAttributeSet::GetPrevLevelExperienceAttribute(), bFound);
		S.NextLevelExperience = ASC->GetGameplayAttributeValue(UCHeroAttributeSet::GetNextLevelExperienceAttribute(), bFound);
	}

	// --- Generic ranking helper ---
	// Takes a function that reads one stat, and a function that returns a writable reference to the matching rank field. Sorting an array of
	// POINTERS means the ranks get written back into the real entries rather than into throwaway copies.
	auto AssignRanks = [this](TFunctionRef<float(const FPlayerMatchStats&)> GetValue,
		TFunctionRef<int32& (FPlayerMatchStats&)> GetRankRef)
		{
			TArray<FPlayerMatchStats*> Sorted;
			for (FPlayerMatchStats& S : MatchStatsArray) { Sorted.Add(&S); }

			// Descending: highest value first, so index 0 becomes rank 1
			Sorted.Sort([&GetValue](const FPlayerMatchStats& A, const FPlayerMatchStats& B)
				{ return GetValue(A) > GetValue(B); });

			// Convert array index to a 1-based position for display
			for (int32 i = 0; i < Sorted.Num(); ++i) { GetRankRef(*Sorted[i]) = i + 1; }
		};

	// The four displayed stats, each ranked independently
	AssignRanks([](const FPlayerMatchStats& S) { return (float)S.HeroKills; }, [](FPlayerMatchStats& S) -> int32& { return S.RankKills; });
	AssignRanks([](const FPlayerMatchStats& S) { return S.GetKD(); }, [](FPlayerMatchStats& S) -> int32& { return S.RankKD; });
	AssignRanks([](const FPlayerMatchStats& S) { return S.CaptureSeconds; }, [](FPlayerMatchStats& S) -> int32& { return S.RankCapture; });
	AssignRanks([](const FPlayerMatchStats& S) { return S.DamageInflicted; }, [](FPlayerMatchStats& S) -> int32& { return S.RankDamage; });

	// Overall rank is XP — rank 1 is the MVP
	AssignRanks([](const FPlayerMatchStats& S) { return S.Experience; }, [](FPlayerMatchStats& S) -> int32& { return S.RankOverall; });

	// Submit to the leaderboard backend now - a player who quits from the stats screen (or crashes) must not be able to dodge their result
	SubmitMatchResultsToLeaderboard();

	// Tell any listeners on the server; clients get told by OnRep instead
	OnMatchStatsUpdated.Broadcast(MatchStatsArray);
}

// Fires on clients when the array replicates down, which is the signal the stats screen waits for before it has anything to display
void AChrisGameState::OnRep_MatchStatsArray()
{
	OnMatchStatsUpdated.Broadcast(MatchStatsArray);
}