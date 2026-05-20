// Christopher Naglik All Rights Reserved


#include "Player/ChrisPlayerController.h"
#include "Player/ChrisPlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/GameplayWidget.h"
#include "Widgets/MatchCountdownWidget.h"
#include "Widgets/RoundTimerWidget.h"
#include "Widgets/ShopWidget.h"
#include "Framework/ChrisGameMode.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/SpringArmComponent.h"



void AChrisPlayerController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);
	ChrisPlayerCharacter = Cast<AChrisPlayerCharacter>(NewPawn);
	if (ChrisPlayerCharacter)
	{
		ChrisPlayerCharacter->ServerSideInit();
		ChrisPlayerCharacter->SetGenericTeamId(TeamID);
	}
}

void AChrisPlayerController::AcknowledgePossession(APawn* NewPawn)
{
	Super::AcknowledgePossession(NewPawn);
	ChrisPlayerCharacter = Cast<AChrisPlayerCharacter>(NewPawn);

	if (ChrisPlayerCharacter)
	{
		ChrisPlayerCharacter->ClientSideInit();
		SpawnGameplayWidget();

		// Disable input immediately
		ChrisPlayerCharacter->DisableInput(this);

		// Hide gameplay widget
		if (GameplayWidget)
		{
			GameplayWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void AChrisPlayerController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID = NewTeamID;
}

FGenericTeamId AChrisPlayerController::GetGenericTeamId() const
{
	return TeamID;
}

void AChrisPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AChrisPlayerController, TeamID);
}

void AChrisPlayerController::SpawnGameplayWidget()
{
	if (!IsLocalPlayerController())
		return;

	GameplayWidget = CreateWidget <UGameplayWidget> (this, GameplayWidgetClass);
	if(GameplayWidget)
	{
		GameplayWidget->AddToViewport();
		GameplayWidget->ConfigureAbilities(ChrisPlayerCharacter->GetAbilities());
	}
}

// ============================================================
// COUNTDOWN RPCs
// ============================================================

void AChrisPlayerController::Client_OnCountdownStart_Implementation(int32 Seconds)
{
	// Disable all input on the character (movement, abilities, everything).
	if (ChrisPlayerCharacter)
	{
		ChrisPlayerCharacter->DisableInput(this);
	}

	// Hide the gameplay widget during countdown.
	if (GameplayWidget)
	{
		GameplayWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Create the countdown widget and show it.
	// Z-order 100 puts it on top of everything else.
	if (CountdownWidgetClass)
	{
		CountdownWidget = CreateWidget<UMatchCountdownWidget>(this, CountdownWidgetClass);
		if (CountdownWidget)
		{
			CountdownWidget->AddToViewport(100);
			CountdownWidget->UpdateCountdown(Seconds);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[Client] Countdown started: %d"), Seconds);
}

// Server calls this every second → updates the number on screen.
void AChrisPlayerController::Client_OnCountdownTick_Implementation(int32 SecondsRemaining)
{
	if (CountdownWidget)
	{
		CountdownWidget->UpdateCountdown(SecondsRemaining);
	}

	UE_LOG(LogTemp, Log, TEXT("[Client] Countdown tick: %d"), SecondsRemaining);
}

// ROUND START RPC
void AChrisPlayerController::Client_OnRoundStart_Implementation(float Duration)
{
	// Re-enable input so the player can move and use abilities.
	if (ChrisPlayerCharacter)
	{
		ChrisPlayerCharacter->EnableInput(this);
	}

	// Remove the countdown widget — it's no longer needed.
	// RemoveFromParent() removes it from the viewport and allows garbage collection.
	if (CountdownWidget)
	{
		CountdownWidget->RemoveFromParent();
		CountdownWidget = nullptr;
	}

	// Show the gameplay widget (health bars, stats, abilities).
	if (GameplayWidget)
	{
		GameplayWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	// Create and show the round timer at the top of screen.
	if (RoundTimerWidgetClass)
	{
		RoundTimerWidget = CreateWidget<URoundTimerWidget>(this, RoundTimerWidgetClass);
		if (RoundTimerWidget)
		{
			RoundTimerWidget->AddToViewport(50);
			RoundTimerWidget->StartTimer(Duration);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[Client] Round started! Duration: %.0f"), Duration);
}

// ROUND END RPC
void AChrisPlayerController::Client_OnRoundEnd_Implementation()
{
	if (ChrisPlayerCharacter)
	{
		ChrisPlayerCharacter->DisableInput(this);
	}

	// Remove the round timer
	if (RoundTimerWidget)
	{
		RoundTimerWidget->RemoveFromParent();
		RoundTimerWidget = nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("[Client] Round ended."));
}

// FADE RPC
void AChrisPlayerController::Client_OnFadeToBlack_Implementation(float Duration)
{
	if (PlayerCameraManager)
	{
		PlayerCameraManager->StartCameraFade(0.f, 1.f, Duration, FLinearColor::Black, false, true);
	}

	UE_LOG(LogTemp, Log, TEXT("[Client] Fading to black over %.1fs"), Duration);
}

// SHOP PHASE START RPC
void AChrisPlayerController::Client_OnShopPhaseStart_Implementation(float InShopDuration, float FadeInDuration)
{
	// Hide the gameplay widget (do not destroy - reuse it next round)
	if (GameplayWidget)
	{
		GameplayWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Create and show the shop widget
	if (ShopWidgetClass)
	{
		ShopWidget = CreateWidget<UShopWidget>(this, ShopWidgetClass);
		if (ShopWidget)
		{
			ShopWidget->AddToViewport();
			ShopWidget->StartTimer(InShopDuration);
		}
	}

	SetShowMouseCursor(true);
	SetInputMode(FInputModeGameAndUI());
}

// RETURN TO ARENA RPC
void AChrisPlayerController::Client_OnReturnToArena_Implementation(float FadeInDuration)
{
	// Remove the shop widget
	if (ShopWidget)
	{
		ShopWidget->RemoveFromParent();
		ShopWidget = nullptr;
	}

	// Hide cursor, return to game-only input mode.
	// FInputModeGameOnly means mouse is captured by the game (no visible cursor).
	SetShowMouseCursor(false);
	SetInputMode(FInputModeGameOnly());

	// Show gameplay widget again for the next round
	if (GameplayWidget)
	{
		GameplayWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

// CONTINUE VOTE (Server RPC)
void AChrisPlayerController::Server_VoteContinue_Implementation()
{
	// GetWorld()->GetAuthGameMode() returns the GameMode, which only exists on the server.
	// This is the correct way to access it from a PlayerController.
	AChrisGameMode* GM = GetWorld()->GetAuthGameMode<AChrisGameMode>();
	if (GM)
	{
		GM->OnPlayerVoteContinue(this);
	}
}

// Return false to reject the RPC (e.g., anti-cheat). Accept all for now.
bool AChrisPlayerController::Server_VoteContinue_Validate()
{
	return true;
}

void AChrisPlayerController::Client_OnFadeFromBlack_Implementation(float Duration)
{
	if (PlayerCameraManager)
	{
		PlayerCameraManager->StartCameraFade(1.f, 0.f, Duration, FLinearColor::Black, false, false);
	}
}

void AChrisPlayerController::Client_OnSetShopCamera_Implementation()
{
	if (!ChrisPlayerCharacter) return;

	USpringArmComponent* Boom = ChrisPlayerCharacter->FindComponentByClass<USpringArmComponent>();
	if (!Boom) return;

	// Save current camera settings so we can restore them later
	OriginalBoomRotation = Boom->GetRelativeRotation();
	OriginalBoomSocketOffset = Boom->SocketOffset;
	bOriginalUsePawnControlRotation = Boom->bUsePawnControlRotation;

	Boom->bUsePawnControlRotation = false;

	// Also stop the character itself from inheriting controller yaw
	ChrisPlayerCharacter->bUseControllerRotationYaw = false;

	Boom->SetWorldRotation(FRotator(0.f, ChrisPlayerCharacter->GetActorRotation().Yaw + 170.f, 0.f));

	// Offset camera to the left in screen space
	Boom->SocketOffset = FVector(0.f, -180.f, -10.f);
	OriginalArmLength = Boom->TargetArmLength;
	Boom->TargetArmLength = 240.f;
}

void AChrisPlayerController::Client_OnSetArenaCamera_Implementation()
{
	if (!ChrisPlayerCharacter) return;

	USpringArmComponent* Boom = ChrisPlayerCharacter->FindComponentByClass<USpringArmComponent>();
	if (!Boom) return;

	// Restore original camera settings
	Boom->SetRelativeRotation(OriginalBoomRotation);
	Boom->SocketOffset = OriginalBoomSocketOffset;
	Boom->bUsePawnControlRotation = bOriginalUsePawnControlRotation;
	Boom->TargetArmLength = OriginalArmLength;

	// Re-enable controller rotation on the character
	ChrisPlayerCharacter->bUseControllerRotationYaw = true;
}