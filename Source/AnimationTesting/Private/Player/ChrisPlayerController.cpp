// Christopher Naglik All Rights Reserved


#include "Player/ChrisPlayerController.h"
#include "Player/ChrisPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Widgets/GameplayWidget.h"
#include "Widgets/GameplayMenu.h"
#include "Widgets/MatchCountdownWidget.h"
#include "Widgets/RoundTimerWidget.h"
#include "Widgets/ShopWidget.h"
#include "Framework/ChrisGameMode.h"
#include "Framework/Application/SlateApplication.h"
#include "Weapon/SwordEquipComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/SpringArmComponent.h"
#include "Animation/AnimInstance.h"




void AChrisPlayerController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);
	ChrisPlayerCharacter = Cast<AChrisPlayerCharacter>(NewPawn);
	if (ChrisPlayerCharacter)
	{
		ChrisPlayerCharacter->ServerSideInit();
		ChrisPlayerCharacter->SetGenericTeamId(TeamID);
		InitialSpawnRotation = ChrisPlayerCharacter->GetActorRotation();
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

		// Register UI input mapping context so Esc (menu toggle) always works
		if (ULocalPlayer* LP = GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				InputSubsystem->AddMappingContext(UIInputMapping, 1);
			}
		}

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

void AChrisPlayerController::Client_OnResetRotation_Implementation(FRotator SpawnRotation)
{
	InitialSpawnRotation = SpawnRotation;
	SetControlRotation(SpawnRotation);

	if (ChrisPlayerCharacter)
	{
		ChrisPlayerCharacter->SetActorRotation(SpawnRotation);
	}
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

void AChrisPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComp)
	{
		EnhancedInputComp->BindAction(ToggleGameplayMenuAction, ETriggerEvent::Triggered, this, &AChrisPlayerController::ToggleGameplayMenu);
	}
}

void AChrisPlayerController::ToggleGameplayMenu()
{
	if (bIsGameplayMenuOpen)
	{
		// CLOSE the menu
		if (GameplayMenuWidget)
		{
			GameplayMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		bIsGameplayMenuOpen = false;

		// Restore state based on which phase we're in
		bool bInShop = ShopWidget && ShopWidget->GetVisibility() != ESlateVisibility::Collapsed;
		

		if (bIsRoundActive)
		{
			// Active round: re-enable input, hide cursor
			if (ChrisPlayerCharacter)
			{
				ChrisPlayerCharacter->EnableInput(this);
			}
			SetShowMouseCursor(false);
			SetInputMode(FInputModeGameOnly());
		}
		else if (bInShop)
		{
			// Shop: keep cursor, Game+UI mode
			SetShowMouseCursor(true);
			FInputModeGameAndUI GameAndUI;
			GameAndUI.SetHideCursorDuringCapture(false);
			SetInputMode(GameAndUI);
		}
		else
		{
			// Countdown/round end/transition: hide cursor, don't re-enable input
			SetShowMouseCursor(false);
			SetInputMode(FInputModeGameOnly());
		}
	}
	else
	{
		// OPEN the menu (same as before)
		if (!GameplayMenuWidget && GameplayMenuClass)
		{
			GameplayMenuWidget = CreateWidget<UGameplayMenu>(this, GameplayMenuClass);
			if (GameplayMenuWidget)
			{
				GameplayMenuWidget->AddToViewport(200);
				GameplayMenuWidget->GetReturnToArenaButtonClickedEventDelegate().AddDynamic(this, &AChrisPlayerController::ToggleGameplayMenu);
			}
		}

		if (GameplayMenuWidget)
		{
			GameplayMenuWidget->SetVisibility(ESlateVisibility::Visible);
		}
		bIsGameplayMenuOpen = true;

		if (ChrisPlayerCharacter)
		{
			ChrisPlayerCharacter->DisableInput(this);
		}
		SetShowMouseCursor(true);
		FInputModeGameAndUI GameAndUI;
		GameAndUI.SetHideCursorDuringCapture(false);
		SetInputMode(GameAndUI);
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
	bIsRoundActive = true;

	// Only re-enable input if the gameplay menu is NOT open
	if (!bIsGameplayMenuOpen && ChrisPlayerCharacter)
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
		GameplayWidget->ResetAllCooldowns();
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
	bIsRoundActive = false;

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

	// Force character back to idle animation (stop any active montages)
	if (ChrisPlayerCharacter)
	{
		if (UAnimInstance* AnimInstance = ChrisPlayerCharacter->GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Stop(0.25f);  // Quick blend to idle
		}
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
	if (GameplayWidget)
	{
		GameplayWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Create the shop widget only the first time
	if (!ShopWidget && ShopWidgetClass)
	{
		ShopWidget = CreateWidget<UShopWidget>(this, ShopWidgetClass);
		if (ShopWidget)
		{
			ShopWidget->AddToViewport();
		}
	}

	// Show it and restart the timer
	if (ShopWidget)
	{
		ShopWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		ShopWidget->SetRenderOpacity(0.f);
		ShopWidget->StartTimer(InShopDuration);

		// Fade the widget in over the same duration as the camera fade
		GetWorldTimerManager().SetTimer(ShopFadeTimerHandle, [this]()
			{
				if (ShopWidget)
				{
					float Current = ShopWidget->GetRenderOpacity();
					float NewOpacity = FMath::Clamp(Current + 0.005f, 0.f, 1.f);
					ShopWidget->SetRenderOpacity(NewOpacity);

					if (NewOpacity >= 1.f)
					{
						GetWorldTimerManager().ClearTimer(ShopFadeTimerHandle);
					}
				}
			}, 0.016f, true);
	}
	SetShowMouseCursor(true);
	SetInputMode(FInputModeGameAndUI());
}

void AChrisPlayerController::Client_OnReturnToArena_Implementation(float FadeInDuration)
{
	// Reset weapons to sheathed on client
	if (ChrisPlayerCharacter)
	{
		if (USwordEquipComponent* SwordComponent = ChrisPlayerCharacter->FindComponentByClass<USwordEquipComponent>())
		{
			SwordComponent->ResetToUnequipped();
		}
	}
	
	// Force-close menu if it was open during shop
	if (bIsGameplayMenuOpen)
	{
		if (GameplayMenuWidget)
		{
			GameplayMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		bIsGameplayMenuOpen = false;
	}

	if (ShopWidget)
	{
		GetWorldTimerManager().SetTimer(ShopFadeTimerHandle, [this]()
			{
				if (ShopWidget)
				{
					float Current = ShopWidget->GetRenderOpacity();
					float NewOpacity = FMath::Clamp(Current - 0.005f, 0.f, 1.f);
					ShopWidget->SetRenderOpacity(NewOpacity);

					if (NewOpacity <= 0.f)
					{
						GetWorldTimerManager().ClearTimer(ShopFadeTimerHandle);
						ShopWidget->SetVisibility(ESlateVisibility::Collapsed);
					}
				}
			}, 0.016f, true);
	}

	SetShowMouseCursor(false);
	SetInputMode(FInputModeGameOnly());

	// Explicitly grab focus for the game viewport
	FSlateApplication::Get().SetAllUserFocusToGameViewport();
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

	// Save current camera settings
	OriginalBoomRotation = Boom->GetRelativeRotation();
	OriginalBoomSocketOffset = Boom->SocketOffset;
	bOriginalUsePawnControlRotation = Boom->bUsePawnControlRotation;

	// Decouple CAMERA from controller (for shop preview angle)
	Boom->bUsePawnControlRotation = false;

	// Keep character driven by controller rotation — this is what makes
	// the rotation reset actually stick (same as arena camera)
	ChrisPlayerCharacter->bUseControllerRotationYaw = true;
	ChrisPlayerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
	ChrisPlayerCharacter->GetCharacterMovement()->StopMovementImmediately();

	// Force control rotation to spawn direction
	SetControlRotation(InitialSpawnRotation);

	// Position camera independently at the shop preview angle
	Boom->SetWorldRotation(FRotator(0.f, InitialSpawnRotation.Yaw + 170.f, 0.f));

	Boom->SocketOffset = FVector(0.f, -180.f, -10.f);
	OriginalArmLength = Boom->TargetArmLength;
	Boom->TargetArmLength = 240.f;
}

void AChrisPlayerController::Client_OnSetArenaCamera_Implementation()
{
	if (!ChrisPlayerCharacter) return;

	USpringArmComponent* Boom = ChrisPlayerCharacter->FindComponentByClass<USpringArmComponent>();
	if (!Boom) return;

	Boom->SetRelativeRotation(OriginalBoomRotation);
	Boom->SocketOffset = OriginalBoomSocketOffset;
	Boom->bUsePawnControlRotation = bOriginalUsePawnControlRotation;
	Boom->TargetArmLength = OriginalArmLength;

	// Restore movement rotation control
	ChrisPlayerCharacter->GetCharacterMovement()->bOrientRotationToMovement = true;
	ChrisPlayerCharacter->bUseControllerRotationYaw = true;
}