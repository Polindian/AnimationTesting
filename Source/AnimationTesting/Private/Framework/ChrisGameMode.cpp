// Christopher Naglik All Rights Reserved


#include "Framework/ChrisGameMode.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Player/ChrisPlayerController.h"
#include "Player/ChrisPlayerCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "GAS/ChrisAbilitySystemComponent.h"
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
	int32 FlagCount = 0;
	for (TActorIterator<AFlag> It(GetWorld()); It; ++It)
	{
		FlagCount++;
		UE_LOG(LogTemp, Warning, TEXT("Found flag: %s | BannerEffect: %s"),
			*It->GetName(),
			It->FindComponentByClass<UNiagaraComponent>() ? TEXT("VALID") : TEXT("NULL"));
		It->SetCaptureEnabled(true);
		It->PlayBannerSlam(FLinearColor::White);
	}
	UE_LOG(LogTemp, Warning, TEXT("[MatchFlow] Total flags found: %d"), FlagCount);
}

// ROUND_END phase: 5 second pause, input disabled
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

	ForEachPlayerController([](AChrisPlayerController* PC)
		{
			PC->Client_OnRoundEnd();
		});

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

	// Screen is fully black. Restart AI, swap widgets, restore camera.
	StartAllAISpawning();

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
	for (TActorIterator<ASkeletonBarrack> It(GetWorld()); It; ++It)
	{
		It->StartSpawning();
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
			if (PC->StartSpot.IsValid() && PC->GetPawn())
			{
				FVector Location = PC->StartSpot->GetActorLocation();
				FRotator Rotation = PC->StartSpot->GetActorRotation();
				PC->GetPawn()->TeleportTo(Location, Rotation);
				PC->ClientSetRotation(Rotation, true);
			}
		});
}