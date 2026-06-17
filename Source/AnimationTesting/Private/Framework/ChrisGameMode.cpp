// Christopher Naglik All Rights Reserved


#include "Framework/ChrisGameMode.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AI/ChrisAIController.h"
#include "AI/SkeletonBarrack.h"
#include "Player/ChrisPlayerController.h"
#include "Player/ChrisPlayerCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "GAS/ChrisAbilitySystemComponent.h"
#include "GAS/CHeroAttributeSet.h"
#include "EngineUtils.h"
#include "Framework/Flag.h"
#include "AI/SkeletonBarrack.h"
#include "Weapon/SwordEquipComponent.h" 


APlayerController* AChrisGameMode::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
	APlayerController* NewPlayerController = Super::SpawnPlayerController(InRemoteRole, Options);
	IGenericTeamAgentInterface* NewPlayerTeamInterface = Cast<IGenericTeamAgentInterface>(NewPlayerController);
	FGenericTeamId TeamId = GetTeamIDForPlayer(NewPlayerController);
	if (NewPlayerTeamInterface)
	{
		NewPlayerTeamInterface->SetGenericTeamId(TeamId);
	}
	
	NewPlayerController->StartSpot = FindNextStartSpotForTeam(TeamId);
	
	return NewPlayerController;
}

FGenericTeamId AChrisGameMode::GetTeamIDForPlayer(const APlayerController* PlayerController) const
{
	static int PlayerCount = 0;
	++PlayerCount;
	return FGenericTeamId(PlayerCount % 2);
}

AActor* AChrisGameMode::FindNextStartSpotForTeam(const FGenericTeamId& TeamID)
{
	const FName* StartSpotTag = TeamStartSpotTagMap.Find(TeamID);
	if (!StartSpotTag)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();

	for(TActorIterator<APlayerStart> It(World); It; ++It)
	{
		if (It->PlayerStartTag == *StartSpotTag)
		{
			It->PlayerStartTag = FName("Taken");
			return *It;
		}
	}
	return nullptr;
}

void AChrisGameMode::ForEachPlayerController(TFunctionRef<void(AChrisPlayerController*)> Func)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AChrisPlayerController* PC = Cast<AChrisPlayerController>(It->Get());
		if (PC)
		{
			Func(PC);
		}
	}
}

// PostLogin: detect when enough players join to start
void AChrisGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 CurrentPlayerCount = GetNumPlayers();
	UE_LOG(LogTemp, Log, TEXT("[MatchFlow] Player joined. Count: %d / %d"), CurrentPlayerCount, ExpectedPlayerCount);

	// Only start if we have enough players AND we haven't started yet.
	if (CurrentPlayerCount >= ExpectedPlayerCount && CurrentPhase == EMatchPhase::WaitingForPlayers)
	{
		// 2 second delay: lets the last player's pawn fully initialize
		// (ServerSideInit, widget spawning, etc.) before we start the countdown.
		GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &AChrisGameMode::StartCountdown, 2.f, false);
	}
}

// Countdown Timer
void AChrisGameMode::StartCountdown()
{
	CurrentPhase = EMatchPhase::Countdown;
	CountdownSecondsRemaining = (int32)CountdownTime; // starts at 5

	UE_LOG(LogTemp, Log, TEXT("[MatchFlow] Countdown started: %d seconds"), CountdownSecondsRemaining);

	// Disables all player input during countdown.
	ForEachPlayerController([this](AChrisPlayerController* PC)
		{
			PC->Client_OnCountdownStart(CountdownSecondsRemaining);
		});

	// Fire CountdownTick every 1 second. "true" = repeating timer.
	GetWorldTimerManager().SetTimer(CountdownTickHandle, this, &AChrisGameMode::CountdownTick, 1.f, true);
}

void AChrisGameMode::CountdownTick()
{
	CountdownSecondsRemaining--;

	if (CountdownSecondsRemaining > 0)
	{
		// Still counting: "4", "3", "2", "1"
		UE_LOG(LogTemp, Log, TEXT("[MatchFlow] Countdown: %d"), CountdownSecondsRemaining);

		ForEachPlayerController([this](AChrisPlayerController* PC)
			{
				PC->Client_OnCountdownTick(CountdownSecondsRemaining);
			});
	}
	else if (CountdownSecondsRemaining == 0)
	{
		// Show "FIGHT!" on client side
		UE_LOG(LogTemp, Log, TEXT("[MatchFlow] FIGHT!"));

		ForEachPlayerController([](AChrisPlayerController* PC)
			{
				PC->Client_OnCountdownTick(0);
			});
	}
	else
	{
		// Stop the tick timer and start the round.
		GetWorldTimerManager().ClearTimer(CountdownTickHandle);
		StartRound();
	}
}

// IN-ROUND phase: active gameplay for 60 seconds with input and gameplay widget activation
// Schedules EndRound after RoundDuration seconds.
void AChrisGameMode::StartRound()
{
	CurrentPhase = EMatchPhase::InRound;
	CurrentRound++;

	// === NEW === Reset per-round kill counters
	TeamOneKills = 0;
	TeamTwoKills = 0;

	UE_LOG(LogTemp, Log, TEXT("[MatchFlow] Round %d started! Duration: %.0fs"), CurrentRound, RoundDuration);

	ForEachPlayerController([](AChrisPlayerController* PC)
		{
			if (APawn* Pawn = PC->GetPawn())
			{
				if (USwordEquipComponent* SwordComponent = Pawn->FindComponentByClass<USwordEquipComponent>())
				{
					SwordComponent->ResetToUnequipped();
				}
			}
		});

	ForEachPlayerController([](AChrisPlayerController* PC)
		{
			if (APawn* Pawn = PC->GetPawn())
			{
				if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Pawn))
				{
					if (UChrisAbilitySystemComponent* ASC = Cast<UChrisAbilitySystemComponent>(ASI->GetAbilitySystemComponent()))
					{
						ASC->ResetAllCooldowns();
						ASC->ApplyHeavyAbilityCooldowns();
						ASC->ApplyFullStatEffect();
					}
				}
			}
		});

	// Notify all clients: "enable input, show gameplay UI, start your timer display"
	ForEachPlayerController([this](AChrisPlayerController* PC)
		{
			PC->Client_OnRoundStart(RoundDuration);
		});

	// After RoundDuration seconds, end the round.
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &AChrisGameMode::EndRound, RoundDuration, false);

	// Enable flag capturing for this round and white banner slam animation
	// === NEW === Also bind to OnFlagCaptured to detect when all flags are captured early
	int32 FlagCount = 0;
	for (TActorIterator<AFlag> It(GetWorld()); It; ++It)
	{
		FlagCount++;
		It->SetCaptureEnabled(true);
		It->PlayBannerSlam(FLinearColor::White);

		// Bind to each flag's captured event
		It->OnFlagCaptured.AddUObject(this, &AChrisGameMode::OnFlagCapturedCallback);
	}
	UE_LOG(LogTemp, Warning, TEXT("[MatchFlow] Total flags found: %d"), FlagCount);

	StartAllAISpawning();
}

// ROUND_END phase: Determines round winner using full match conditions, awards souls, checks for match end
void AChrisGameMode::EndRound()
{
	CurrentPhase = EMatchPhase::RoundEnd;

	UE_LOG(LogTemp, Log, TEXT("[MatchFlow] Round %d ended. Waiting %.0fs before shop transition."), CurrentRound, RoundEndWaitTime);

	// Cancel ALL active abilities on all players FIRST (stops aim, attacks, etc.)
	ForEachPlayerController([](AChrisPlayerController* PC)
		{
			if (APawn* Pawn = PC->GetPawn())
			{
				if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Pawn))
				{
					if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
					{
						ASC->CancelAllAbilities();
					}
				}
			}
		});

	// Clean up any players who are mid-death
	ForEachPlayerController([](AChrisPlayerController* PC)
		{
			if (AChrisPlayerCharacter* Character = Cast<AChrisPlayerCharacter>(PC->GetPawn()))
			{
				Character->CancelDeathTimers();
				if (Character->IsDead())
				{
					Character->ForceResetFromDeath();
				}
			}
		});

	ForEachPlayerController([](AChrisPlayerController* PC)
		{
			PC->Client_OnRoundEnd();
		});

	// === NEW === Unbind flag captured delegates (prevent stale bindings next round)
	for (TActorIterator<AFlag> It(GetWorld()); It; ++It)
	{
		It->OnFlagCaptured.RemoveAll(this);
	}

	// Award soul points and track round wins based on match conditions
	AwardRoundEndSouls();

	// Check if the match is over (a team reached RoundsToWin)
	if (TeamOneRoundWins >= RoundsToWin)
	{
		EndMatch(true);
		return;
	}
	else if (TeamTwoRoundWins >= RoundsToWin)
	{
		EndMatch(false);
		return;
	}

	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &AChrisGameMode::StartTransitionToShop, RoundEndWaitTime, false);

	// Stop all flag capturing
	for (TActorIterator<AFlag> It(GetWorld()); It; ++It)
	{
		It->SetCaptureEnabled(false);
	}

	// Reset zone time accumulators for all players
	ForEachPlayerController([](AChrisPlayerController* PC)
		{
			if (AChrisPlayerCharacter* Hero = Cast<AChrisPlayerCharacter>(PC->GetPawn()))
			{
				Hero->ResetZoneTimeAccumulator();
			}
		});

	StopAllAISpawning();

	// Stop all AI behavior so they idle during round end
	for (TActorIterator<AChrisAIController> It(GetWorld()); It; ++It)
	{
		It->StopAIBehavior();
	}

	StopAllAIBehavior();
}

// === NEW === Uses DetermineRoundWinner to evaluate all match conditions,
// then awards 50 souls to winners and 100 souls to losers (catch-up mechanic).
// If round is a draw (all tiebreakers failed), no round win is awarded.
void AChrisGameMode::AwardRoundEndSouls()
{
	EFlagOwnership RoundWinner = DetermineRoundWinner();

	if (RoundWinner == EFlagOwnership::Neutral)
	{
		// All tiebreakers failed — round restarts, no winner
		UE_LOG(LogTemp, Warning, TEXT("[MatchFlow] Round %d is a DRAW. Round will be repeated. Both teams get %.0f souls."), CurrentRound, LoserSoulReward);
	}
	else if (RoundWinner == EFlagOwnership::TeamOne)
	{
		TeamOneRoundWins++;
		UE_LOG(LogTemp, Warning, TEXT("[MatchFlow] TEAM ONE wins round %d! (Score: %d-%d)"), CurrentRound, TeamOneRoundWins, TeamTwoRoundWins);
	}
	else
	{
		TeamTwoRoundWins++;
		UE_LOG(LogTemp, Warning, TEXT("[MatchFlow] TEAM TWO wins round %d! (Score: %d-%d)"), CurrentRound, TeamOneRoundWins, TeamTwoRoundWins);
	}

	// Award souls to each player based on whether their team won or lost
	ForEachPlayerController([&](AChrisPlayerController* PC)
		{
			if (!PC || !PC->GetPawn()) return;

			IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PC->GetPawn());
			if (!ASI) return;

			UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
			if (!ASC) return;

			IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(PC);
			if (!TeamAgent) return;

			FGenericTeamId PlayerTeam = TeamAgent->GetGenericTeamId();
			// Team ID 0 = TeamOne (flag system), Team ID 1 = TeamTwo (flag system)
			bool bIsTeamOne = (PlayerTeam.GetId() == 0);

			float Reward = 0.f;
			if (RoundWinner == EFlagOwnership::Neutral)
			{
				// Draw — both teams get loser reward
				Reward = LoserSoulReward;
			}
			else if ((bIsTeamOne && RoundWinner == EFlagOwnership::TeamOne) ||
				(!bIsTeamOne && RoundWinner == EFlagOwnership::TeamTwo))
			{
				Reward = WinnerSoulReward; // Winners get 50
			}
			else
			{
				Reward = LoserSoulReward; // Losers get 100 (catch-up)
			}

			ASC->ApplyModToAttribute(UCHeroAttributeSet::GetSoulAttribute(), EGameplayModOp::Additive, Reward);

			UE_LOG(LogTemp, Log, TEXT("[MatchFlow] Player %s (Team %d) awarded %.0f souls"),
				*PC->GetName(), PlayerTeam.GetId(), Reward);
		});
}

EFlagOwnership AChrisGameMode::DetermineRoundWinner()
{
	int32 TeamOneCapturedFlags = 0;
	int32 TeamTwoCapturedFlags = 0;
	int32 TotalFlags = 0;

	// Territory accumulators for uncaptured flags
	float TeamOneTerritoryOnUncaptured = 0.f;
	float TeamTwoTerritoryOnUncaptured = 0.f;

	// Territory accumulators for ALL flags (used when none are captured)
	float TeamOneTotalTerritory = 0.f;
	float TeamTwoTotalTerritory = 0.f;

	for (TActorIterator<AFlag> It(GetWorld()); It; ++It)
	{
		TotalFlags++;

		if (It->IsCaptured())
		{
			// Count captured flags per team
			if (It->GetOwnership() == EFlagOwnership::TeamOne)
				TeamOneCapturedFlags++;
			else if (It->GetOwnership() == EFlagOwnership::TeamTwo)
				TeamTwoCapturedFlags++;
		}
		else
		{
			// Uncaptured flag — read territory percentage
			float Percent = It->GetCapturePercent();
			if (It->GetOwnership() == EFlagOwnership::TeamOne)
				TeamOneTerritoryOnUncaptured += Percent;
			else if (It->GetOwnership() == EFlagOwnership::TeamTwo)
				TeamTwoTerritoryOnUncaptured += Percent;
		}

		// Total territory for all flags (captured = 100 for that team)
		if (It->IsCaptured())
		{
			if (It->GetOwnership() == EFlagOwnership::TeamOne)
				TeamOneTotalTerritory += 100.f;
			else
				TeamTwoTotalTerritory += 100.f;
		}
		else
		{
			float Percent = It->GetCapturePercent();
			if (It->GetOwnership() == EFlagOwnership::TeamOne)
				TeamOneTotalTerritory += Percent;
			else if (It->GetOwnership() == EFlagOwnership::TeamTwo)
				TeamTwoTotalTerritory += Percent;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[MatchFlow] Round Results — Captured: T1=%d T2=%d | Territory(uncaptured): T1=%.1f T2=%.1f | Territory(total): T1=%.1f T2=%.1f | Kills: T1=%d T2=%d"),
		TeamOneCapturedFlags, TeamTwoCapturedFlags,
		TeamOneTerritoryOnUncaptured, TeamTwoTerritoryOnUncaptured,
		TeamOneTotalTerritory, TeamTwoTotalTerritory,
		TeamOneKills, TeamTwoKills);

	// --- CONDITION 1: Team with more captured flags wins ---
	if (TeamOneCapturedFlags > TeamTwoCapturedFlags)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MatchFlow] Winner decided by: MORE FLAGS CAPTURED (Team One)"));
		return EFlagOwnership::TeamOne;
	}
	if (TeamTwoCapturedFlags > TeamOneCapturedFlags)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MatchFlow] Winner decided by: MORE FLAGS CAPTURED (Team Two)"));
		return EFlagOwnership::TeamTwo;
	}

	// --- CONDITION 2: Tied captures (e.g. 1:1) — check territory on remaining uncaptured flag(s) ---
	if (TeamOneCapturedFlags > 0 && TeamOneCapturedFlags == TeamTwoCapturedFlags)
	{
		if (TeamOneTerritoryOnUncaptured > TeamTwoTerritoryOnUncaptured)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MatchFlow] Winner decided by: TERRITORY ON REMAINING FLAG (Team One)"));
			return EFlagOwnership::TeamOne;
		}
		if (TeamTwoTerritoryOnUncaptured > TeamOneTerritoryOnUncaptured)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MatchFlow] Winner decided by: TERRITORY ON REMAINING FLAG (Team Two)"));
			return EFlagOwnership::TeamTwo;
		}
		// Territory on remaining flag is also equal — fall through to kills
	}

	// --- CONDITION 3: No flags captured — team with highest total territory wins ---
	if (TeamOneCapturedFlags == 0 && TeamTwoCapturedFlags == 0)
	{
		if (TeamOneTotalTerritory > TeamTwoTotalTerritory)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MatchFlow] Winner decided by: HIGHEST TOTAL TERRITORY (Team One)"));
			return EFlagOwnership::TeamOne;
		}
		if (TeamTwoTotalTerritory > TeamOneTotalTerritory)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MatchFlow] Winner decided by: HIGHEST TOTAL TERRITORY (Team Two)"));
			return EFlagOwnership::TeamTwo;
		}
		// Total territory is equal — fall through to kills
	}

	// --- CONDITION 4: Tiebreaker — team with most kills wins ---
	if (TeamOneKills > TeamTwoKills)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MatchFlow] Winner decided by: MOST KILLS (Team One)"));
		return EFlagOwnership::TeamOne;
	}
	if (TeamTwoKills > TeamOneKills)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MatchFlow] Winner decided by: MOST KILLS (Team Two)"));
		return EFlagOwnership::TeamTwo;
	}

	// --- CONDITION 5: Everything is equal — round restarts ---
	UE_LOG(LogTemp, Warning, TEXT("[MatchFlow] ALL TIEBREAKERS FAILED. Round will be restarted."));
	return EFlagOwnership::Neutral;
}

void AChrisGameMode::OnFlagCapturedCallback(EFlagOwnership CapturedByTeam)
{
	if (CurrentPhase != EMatchPhase::InRound) return;

	int32 TeamOneCaps = 0;
	int32 TeamTwoCaps = 0;
	int32 TotalFlags = 0;

	for (TActorIterator<AFlag> It(GetWorld()); It; ++It)
	{
		TotalFlags++;
		if (It->IsCaptured())
		{
			if (It->GetOwnership() == EFlagOwnership::TeamOne)
				TeamOneCaps++;
			else if (It->GetOwnership() == EFlagOwnership::TeamTwo)
				TeamTwoCaps++;
		}
	}

	int32 MajorityNeeded = (TotalFlags / 2) + 1;

	if (TeamOneCaps >= MajorityNeeded || TeamTwoCaps >= MajorityNeeded)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MatchFlow] MAJORITY CAPTURED (T1=%d T2=%d of %d) — ending round early."), TeamOneCaps, TeamTwoCaps, TotalFlags);
		GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
		EndRound();
	}
	else if (TeamOneCaps + TeamTwoCaps == TotalFlags)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MatchFlow] ALL FLAGS CAPTURED — ending round early."));
		GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
		EndRound();
	}
}

// === NEW === Public function to track kills per round.
// KillerTeam is the team ID of the player/AI that got the kill.
void AChrisGameMode::ReportKill(FGenericTeamId KillerTeam)
{
	// Team ID 0 = TeamOne in flag system, Team ID 1 = TeamTwo
	if (KillerTeam.GetId() == 0)
	{
		TeamOneKills++;
		UE_LOG(LogTemp, Log, TEXT("[MatchFlow] Kill reported: Team One (Total: %d)"), TeamOneKills);
	}
	else if (KillerTeam.GetId() == 1)
	{
		TeamTwoKills++;
		UE_LOG(LogTemp, Log, TEXT("[MatchFlow] Kill reported: Team Two (Total: %d)"), TeamTwoKills);
	}
}

// === NEW === Called when a team reaches RoundsToWin.
// Logs the match result and prevents further rounds from starting.
void AChrisGameMode::EndMatch(bool bTeamOneWon)
{
	CurrentPhase = EMatchPhase::MatchOver;

	UE_LOG(LogTemp, Warning, TEXT("========================================"));
	UE_LOG(LogTemp, Warning, TEXT("[MatchFlow] MATCH OVER! %s WINS! (Score: %d-%d)"),
		bTeamOneWon ? TEXT("TEAM ONE") : TEXT("TEAM TWO"), TeamOneRoundWins, TeamTwoRoundWins);
	UE_LOG(LogTemp, Warning, TEXT("========================================"));

	// Stop all flag capturing
	for (TActorIterator<AFlag> It(GetWorld()); It; ++It)
	{
		It->SetCaptureEnabled(false);
	}

	// Stop AI
	StopAllAISpawning();
	DestroyAllAI();

	// Notify all clients that the match is over
	// TODO: Add a Client_OnMatchEnd RPC to show a results screen
	ForEachPlayerController([](AChrisPlayerController* PC)
		{
			PC->Client_OnRoundEnd(); // Disables input for now
		});

	// Reset zone time accumulators
	ForEachPlayerController([](AChrisPlayerController* PC)
		{
			if (AChrisPlayerCharacter* Hero = Cast<AChrisPlayerCharacter>(PC->GetPawn()))
			{
				Hero->ResetZoneTimeAccumulator();
			}
		});
}


// TRANSITION TO SHOP: 3 second fade (1.5s out + 1.5s in)
void AChrisGameMode::StartTransitionToShop()
{
	CurrentPhase = EMatchPhase::TransitionToShop;
	float HalfTransition = TransitionDuration / 2.f;

	UE_LOG(LogTemp, Log, TEXT("[MatchFlow] Fading to black (%.1fs) for shop transition"), HalfTransition);

	// Tell clients to fade their camera to black over HalfTransition seconds.
	ForEachPlayerController([HalfTransition](AChrisPlayerController* PC)
		{
			PC->Client_OnFadeToBlack(HalfTransition);
		});

	// At the midpoint, the screen is fully black — safe to swap widgets.
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &AChrisGameMode::OnTransitionToShopMidpoint, HalfTransition, false);
}

void AChrisGameMode::OnTransitionToShopMidpoint()
{
	StopAllAISpawning();
	DestroyAllAI();
	TeleportPlayersToStart();

	for (TActorIterator<AFlag> It(GetWorld()); It; ++It)
	{
		It->DismissBanner();
	}

	float HalfTransition = TransitionDuration / 2.f;

	UE_LOG(LogTemp, Log, TEXT("[MatchFlow] Screen black. Swapping to shop UI, fading in (%.1fs)"), HalfTransition);

	// Client will: remove GameplayWidget, remove RoundTimer, spawn ShopWidget, fade from black
	ForEachPlayerController([this, HalfTransition](AChrisPlayerController* PC)
		{
			PC->Client_OnShopPhaseStart(ShopDuration, HalfTransition);
			PC->Client_OnSetShopCamera();
		});

	// Hold the black screen, then fade in
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &AChrisGameMode::OnTransitionToShopFadeIn, BlackScreenHoldDuration, false);
}



void AChrisGameMode::OnTransitionToShopFadeIn()
{
	float HalfTransition = TransitionDuration / 2.f;

	// Tell clients to fade from black
	ForEachPlayerController([HalfTransition](AChrisPlayerController* PC)
		{
			PC->Client_OnFadeFromBlack(HalfTransition);
		});

	// After fade-in, officially start shop timer
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &AChrisGameMode::StartShopPhase, HalfTransition, false);
}

// SHOP phase: 60 seconds, or until all players vote Continue
void AChrisGameMode::StartShopPhase()
{
	CurrentPhase = EMatchPhase::ShopPhase;
	ContinueVoters.Empty(); // Reset votes from previous round

	UE_LOG(LogTemp, Log, TEXT("[MatchFlow] Shop phase started. Duration: %.0fs"), ShopDuration);

	// After ShopDuration, auto-transition even if nobody voted.
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &AChrisGameMode::StartTransitionToArena, ShopDuration, false);
}

// If all players have voted, skip remaining shop time immediately.
void AChrisGameMode::OnPlayerVoteContinue(APlayerController* VotingPlayer)
{
	// Only accept votes during shop phase
	if (CurrentPhase != EMatchPhase::ShopPhase) return;

	// TSet::Add returns false if already present — prevents double voting
	bool bAlreadyInSet = false;
	ContinueVoters.Add(VotingPlayer, &bAlreadyInSet);

	if (bAlreadyInSet) return; // Already voted, ignore

	UE_LOG(LogTemp, Log, TEXT("[MatchFlow] Continue vote: %d / %d"), ContinueVoters.Num(), ExpectedPlayerCount);

	if (ContinueVoters.Num() >= ExpectedPlayerCount)
	{
		// Everyone voted — skip remaining shop time
		GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
		StartTransitionToArena();
	}
}

// TRANSITION TO ARENA: same fade logic, but reversed
void AChrisGameMode::StartTransitionToArena()
{
	CurrentPhase = EMatchPhase::TransitionToArena;
	float HalfTransition = TransitionDuration / 2.f;

	UE_LOG(LogTemp, Log, TEXT("[MatchFlow] Fading to black (%.1fs) for arena transition"), HalfTransition);

	ForEachPlayerController([HalfTransition](AChrisPlayerController* PC)
		{
			PC->Client_OnFadeToBlack(HalfTransition);
		});

	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &AChrisGameMode::OnTransitionToArenaMidpoint, HalfTransition, false);
}

// Screen is black. Swap back to arena UI, fade in, then start next round's countdown.
void AChrisGameMode::OnTransitionToArenaMidpoint()
{
	// Reset all flag zones for the new round
	for (TActorIterator<AFlag> It(GetWorld()); It; ++It)
	{
		It->ResetCapture();
	}

	float HalfTransition = TransitionDuration / 2.f;

	UE_LOG(LogTemp, Log, TEXT("[MatchFlow] Screen black. Swapping to arena UI, fading in (%.1fs)"), HalfTransition);

	// Client will: remove ShopWidget, show GameplayWidget, fade from black
	ForEachPlayerController([HalfTransition](AChrisPlayerController* PC)
		{
			PC->Client_OnReturnToArena(HalfTransition);
			PC->Client_OnSetArenaCamera();
		});

	// Hold black screen, then fade in
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &AChrisGameMode::OnTransitionToArenaFadeIn, BlackScreenHoldDuration, false);

}

void AChrisGameMode::OnTransitionToArenaFadeIn()
{
	float HalfTransition = TransitionDuration / 2.f;

	ForEachPlayerController([HalfTransition](AChrisPlayerController* PC)
		{
			PC->Client_OnFadeFromBlack(HalfTransition);
		});

	// After fade-in, start next countdown
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &AChrisGameMode::StartCountdown, HalfTransition, false);
}

void AChrisGameMode::StopAllAISpawning()
{
	for (TActorIterator<ASkeletonBarrack> It(GetWorld()); It; ++It)
	{
		It->StopSpawning();
	}
}

void AChrisGameMode::StartAllAISpawning()
{
	int32 TotalPlayers = GetNumPlayers();
	int32 PlayersPerTeam = FMath::Max(1, TotalPlayers / 2);

	for (TActorIterator<ASkeletonBarrack> It(GetWorld()); It; ++It)
	{
		It->StartSpawning(PlayersPerTeam);
	}
}

void AChrisGameMode::DestroyAllAI()
{
	for (TActorIterator<ASkeletonBarrack> It(GetWorld()); It; ++It)
	{
		It->DestroyAllSkeletons();
	}
}

void AChrisGameMode::TeleportPlayersToStart()
{
	ForEachPlayerController([](AChrisPlayerController* PC)
		{
			if (APawn* Pawn = PC->GetPawn())
			{
				TWeakObjectPtr<AActor> StartSpot = PC->StartSpot;
				if (StartSpot.IsValid())
				{
					FRotator SpawnRotation = StartSpot->GetActorRotation();
					Pawn->SetActorTransform(StartSpot->GetActorTransform());
					PC->SetControlRotation(SpawnRotation); // Server side
					PC->Client_OnResetRotation(SpawnRotation); // Client side
				}
			}
		});
}

void AChrisGameMode::StopAllAIBehavior()
{
	for (TActorIterator<AChrisAIController> It(GetWorld()); It; ++It)
	{
		It->StopAIBehavior();
	}
}

