// Christopher Naglik All Rights Reserved


#include "Player/ChrisPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GAS/ChrisAbilitySystemStatics.h"
#include "GAS/CHeroAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Weapon/SwordEquipComponent.h"

AChrisPlayerCharacter::AChrisPlayerCharacter()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("Camera Boom");
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->bUsePawnControlRotation = true;

	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 10.f;

	ViewCam = CreateDefaultSubobject<UCameraComponent>("View Cam");
	ViewCam->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

	SwordEquipComponent = CreateDefaultSubobject<USwordEquipComponent>(TEXT("SwordEquipComponent"));

	HeroAttributeSet = CreateDefaultSubobject<UCHeroAttributeSet>(TEXT("Hero Attribute Set"));
}

void AChrisPlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
  
	APlayerController* OwningPlayerController = GetController<APlayerController>();
	if (OwningPlayerController)
	{
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem = OwningPlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		if (InputSubsystem)
		{
			InputSubsystem->RemoveMappingContext(GameplayInputMappingContext);
			InputSubsystem->AddMappingContext(GameplayInputMappingContext, 0);
		}
	}
}

void AChrisPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if(EnhancedInputComp)
	{
		EnhancedInputComp->BindAction(JumpInputAction, ETriggerEvent::Triggered, this, &AChrisPlayerCharacter::Jump);
		EnhancedInputComp->BindAction(LookInputAction, ETriggerEvent::Triggered, this, &AChrisPlayerCharacter::HandleLookInput);
		EnhancedInputComp->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &AChrisPlayerCharacter::HandleMoveInput);
		EnhancedInputComp->BindAction(UpgradeAbilityLeaderAction, ETriggerEvent::Started, this, &AChrisPlayerCharacter::UpgradeAbilityLeaderDown);
		EnhancedInputComp->BindAction(UpgradeAbilityLeaderAction, ETriggerEvent::Completed, this, &AChrisPlayerCharacter::UpgradeAbilityLeaderUp);

        for (const TPair<EChrisAbilityInputID,UInputAction*>& InputActionPair : GameplayAbilityInputActions)
        {
            EnhancedInputComp->BindAction(InputActionPair.Value, ETriggerEvent::Triggered, this, &AChrisPlayerCharacter::HandleAbilityInput, InputActionPair.Key);
		}
        
	}
}

void AChrisPlayerCharacter::HandleLookInput(const FInputActionValue& InputActionValue)
{
	FVector2D InputValue = InputActionValue.Get<FVector2D>();
	AddControllerPitchInput(InputValue.Y);
	AddControllerYawInput(InputValue.X);
}

void AChrisPlayerCharacter::HandleMoveInput(const FInputActionValue& InputActionValue)
{
	FVector2D InputValue = InputActionValue.Get<FVector2D>();
	InputValue.Normalize();

	AddMovementInput(GetMoveFwdDir() * InputValue.Y + GetLookRightDir() * InputValue.X);
}

void AChrisPlayerCharacter::HandleAbilityInput(const FInputActionValue& InputActionValue, EChrisAbilityInputID InputID)
{
	UE_LOG(LogTemp, Warning, TEXT("HandleAbilityInput called with InputID: %d"), (int32)InputID);
	bool bPressed = InputActionValue.Get<bool>();

	if (bPressed && bIsUpgradeAbilityLeaderDown)
	{
		UpgradeAbilityWithInputID(InputID);
		return;
	}
	if (bPressed)
	{
		GetAbilitySystemComponent()->AbilityLocalInputPressed((int32)InputID);

		// Send event for right hand input (Attack Slash)
		if (InputID == EChrisAbilityInputID::BasicAttack)
		{
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, UChrisAbilitySystemStatics::GetBasicAttackInputPressedTag(), FGameplayEventData());
			Server_SendGameplayEventToSelf(UChrisAbilitySystemStatics::GetBasicAttackInputPressedTag(), FGameplayEventData());
		}
		
		if (InputID == EChrisAbilityInputID::BasicAttack1)
		{
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, UChrisAbilitySystemStatics::GetBasicAttack1InputPressedTag(), FGameplayEventData());
			Server_SendGameplayEventToSelf(UChrisAbilitySystemStatics::GetBasicAttack1InputPressedTag(), FGameplayEventData());
		}

		// Stab Attacks
		if (InputID == EChrisAbilityInputID::BasicAttack2)
		{
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, UChrisAbilitySystemStatics::GetBasicAttack2InputPressedTag(), FGameplayEventData());
			Server_SendGameplayEventToSelf(UChrisAbilitySystemStatics::GetBasicAttack2InputPressedTag(), FGameplayEventData());
		}

		if (InputID == EChrisAbilityInputID::BasicAttack3)
		{
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, UChrisAbilitySystemStatics::GetBasicAttack3InputPressedTag(), FGameplayEventData());
			Server_SendGameplayEventToSelf(UChrisAbilitySystemStatics::GetBasicAttack3InputPressedTag(), FGameplayEventData());
		}

		// Swipe Attacks
		if (InputID == EChrisAbilityInputID::BasicAttack4)
		{
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, UChrisAbilitySystemStatics::GetBasicAttack4InputPressedTag(), FGameplayEventData());
			Server_SendGameplayEventToSelf(UChrisAbilitySystemStatics::GetBasicAttack4InputPressedTag(), FGameplayEventData());
		}

		if (InputID == EChrisAbilityInputID::BasicAttack5)
		{
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, UChrisAbilitySystemStatics::GetBasicAttack5InputPressedTag(), FGameplayEventData());
			Server_SendGameplayEventToSelf(UChrisAbilitySystemStatics::GetBasicAttack5InputPressedTag(), FGameplayEventData());
		}

		// Dodge Hit Attacks
		if (InputID == EChrisAbilityInputID::BasicAttack6)
		{
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, UChrisAbilitySystemStatics::GetBasicAttack6InputPressedTag(), FGameplayEventData());
			Server_SendGameplayEventToSelf(UChrisAbilitySystemStatics::GetBasicAttack6InputPressedTag(), FGameplayEventData());
		}

		if (InputID == EChrisAbilityInputID::BasicAttack7)
		{
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, UChrisAbilitySystemStatics::GetBasicAttack7InputPressedTag(), FGameplayEventData());
			Server_SendGameplayEventToSelf(UChrisAbilitySystemStatics::GetBasicAttack7InputPressedTag(), FGameplayEventData());
		}

		// Smash
		if (InputID == EChrisAbilityInputID::HeavyAttack)
		{
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, UChrisAbilitySystemStatics::GetHeavyAttackInputPressedTag(), FGameplayEventData());
			Server_SendGameplayEventToSelf(UChrisAbilitySystemStatics::GetHeavyAttackInputPressedTag(), FGameplayEventData());
		}

		// Skill Stun
		if (InputID == EChrisAbilityInputID::HeavyAttack1)
		{
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, UChrisAbilitySystemStatics::GetHeavyAttack1InputPressedTag(), FGameplayEventData());
			Server_SendGameplayEventToSelf(UChrisAbilitySystemStatics::GetHeavyAttack1InputPressedTag(), FGameplayEventData());
		}

		// Aim
		if (InputID == EChrisAbilityInputID::HeavyAttack3Aim)
		{
			FGameplayTag HeavyAttack3Tag = bPressed ? UChrisAbilitySystemStatics::GetHeavyAttack3InputPressedTag() : UChrisAbilitySystemStatics::GetHeavyAttack3InputReleasedTag();
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, HeavyAttack3Tag, FGameplayEventData());
			Server_SendGameplayEventToSelf(HeavyAttack3Tag, FGameplayEventData());
		}

		// Shoot 
		if (InputID == EChrisAbilityInputID::HeavyAttack3Shoot)
		{
			if (bPressed)
			{
				FGameplayTag ShootTag = UChrisAbilitySystemStatics::GetHeavyAttack3ShootInputTag();
				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, ShootTag, FGameplayEventData());
				if (!HasAuthority())
				{
					Server_SendGameplayEventToSelf(ShootTag, FGameplayEventData());
				}
			}
		}

		// Rolling 
		if (InputID == EChrisAbilityInputID::Roll)
		{
			APlayerController* PlayerController = Cast<APlayerController>(GetController());
			if (PlayerController)
			{
				UEnhancedInputLocalPlayerSubsystem* InputSubsystem = PlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
				if (InputSubsystem && InputSubsystem->GetPlayerInput())
				{
					FInputActionValue ActionValue = InputSubsystem->GetPlayerInput()->GetActionValue(MoveInputAction);
					FVector2D MoveInput = ActionValue.Get<FVector2D>();
					MoveInput.Normalize();

					EChrisAbilityInputID RollID = GetRollDirectionFromInput(MoveInput);
					GetAbilitySystemComponent()->AbilityLocalInputPressed((int32)RollID);
				}
			}
			return;
		}
	}
	else
	{
		GetAbilitySystemComponent()->AbilityLocalInputReleased((int32)InputID);
	}
}

void AChrisPlayerCharacter::UpgradeAbilityLeaderDown(const FInputActionValue& InputActionValue)
{
	bIsUpgradeAbilityLeaderDown = true;
}

void AChrisPlayerCharacter::UpgradeAbilityLeaderUp(const FInputActionValue& InputActionValue)
{
	bIsUpgradeAbilityLeaderDown = false;
}

EChrisAbilityInputID AChrisPlayerCharacter::GetRollDirectionFromInput(FVector2D MoveInput) const
{
	if (MoveInput.IsNearlyZero())
		return EChrisAbilityInputID::RollBack;

	float Angle = FMath::Atan2(MoveInput.X, MoveInput.Y) * (180.f / PI);

	if (Angle >= -22.5f && Angle < 22.5f)   return EChrisAbilityInputID::RollForward;
	if (Angle >= 22.5f && Angle < 67.5f)    return EChrisAbilityInputID::RollForwardRight;
	if (Angle >= 67.5f && Angle < 112.5f)   return EChrisAbilityInputID::RollRight;
	if (Angle >= 112.5f && Angle < 157.5f)  return EChrisAbilityInputID::RollBackRight;
	if (Angle >= -67.5f && Angle < -22.5f)  return EChrisAbilityInputID::RollForwardLeft;
	if (Angle >= -112.5f && Angle < -67.5f) return EChrisAbilityInputID::RollLeft;
	if (Angle >= -157.5f && Angle < -112.5f) return EChrisAbilityInputID::RollBackLeft;

	return EChrisAbilityInputID::RollBack;
}


void AChrisPlayerCharacter::SetInputEnabledFromPlayerController(bool bEnabled)
{
	APlayerController* PlayerController = GetController<APlayerController>();
	if (!PlayerController)
	{
		return;
	}

	if (bEnabled)
	{
		EnableInput(PlayerController);
	}

	else
	{
		DisableInput(PlayerController);
	}
}

void AChrisPlayerCharacter::OnDead()
{
	SetInputEnabledFromPlayerController(false);
}

void AChrisPlayerCharacter::OnRespawn()
{
	SetInputEnabledFromPlayerController(true);
}

void AChrisPlayerCharacter::OnStun()
{
	SetInputEnabledFromPlayerController(false);
}

void AChrisPlayerCharacter::OnRecoverFromStun()
{
	if (IsDead()) return;
	SetInputEnabledFromPlayerController(true);
}

void AChrisPlayerCharacter::OnFallBack()
{
	SetInputEnabledFromPlayerController(false);
}

void AChrisPlayerCharacter::OnRecoverFromFallBack()
{
	if (IsDead()) return;
	SetInputEnabledFromPlayerController(true);
}

void AChrisPlayerCharacter::OnAimStateChanged(bool bIsAiming)
{
	CameraBoom->bEnableCameraLag = !bIsAiming;
	LerpCameraToLocalOffsetLocation(bIsAiming ? CameraAimLocalOffset : FVector(0.f));
}

void AChrisPlayerCharacter::LerpCameraToLocalOffsetLocation(const FVector& GoalLocation)
{
	// Store the goal and start a repeating timer.
	// Using a proper looping SetTimer with a handle so ClearTimer actually works.
	// If a lerp is already running, ClearTimer stops it before starting a new one.
	CameraLerpGoal = GoalLocation;
	GetWorldTimerManager().ClearTimer(CameraLerpTimerHandle);
	GetWorldTimerManager().SetTimer(CameraLerpTimerHandle, this, &AChrisPlayerCharacter::TickCameraLocalOffsetLerp, 0.016f, true);
}

void AChrisPlayerCharacter::TickCameraLocalOffsetLerp()
{
	FVector CurrentLocalOffset = ViewCam->GetRelativeLocation();

	// Check if we've reached the goal — if so, snap and STOP
	if (FVector::Dist(CurrentLocalOffset, CameraLerpGoal) < 1.f)
	{
		ViewCam->SetRelativeLocation(CameraLerpGoal);
		GetWorldTimerManager().ClearTimer(CameraLerpTimerHandle);
		return;  // CRITICAL: stop here, don't keep ticking
	}

	// Smooth lerp toward the goal
	float LerpAlpha = FMath::Clamp(GetWorld()->GetDeltaSeconds() * CameraLerpSpeed, 0.f, 1.f);
	FVector NewLocalOffset = FMath::Lerp(CurrentLocalOffset, CameraLerpGoal, LerpAlpha);
	ViewCam->SetRelativeLocation(NewLocalOffset);
}

FVector AChrisPlayerCharacter::GetLookRightDir() const
{
	return ViewCam->GetRightVector();
}

FVector AChrisPlayerCharacter::GetLookFwdDir() const
{
	return ViewCam->GetForwardVector();
}

FVector AChrisPlayerCharacter::GetMoveFwdDir() const
{
	return FVector::CrossProduct(GetLookRightDir(), FVector::UpVector);
}
