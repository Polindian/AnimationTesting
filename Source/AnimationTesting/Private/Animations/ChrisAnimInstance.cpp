// Christopher Naglik All Rights Reserved


#include "Animations/ChrisAnimInstance.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/ChrisCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/ChrisAbilitySystemStatics.h"
#include "Kismet/KismetMathLibrary.h"

void UChrisAnimInstance::NativeInitializeAnimation()
{
	OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
	if(OwnerCharacter)
	{
		OwnerMovementComp = OwnerCharacter->GetCharacterMovement();
	}

	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TryGetPawnOwner());
	if (OwnerASC)
	{
		OwnerASC->RegisterGameplayTagEvent(UChrisAbilitySystemStatics::GetAimStatsTag()).AddUObject(this, &UChrisAnimInstance::OwnerAimTagChanged);
	}
}

void UChrisAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if (OwnerCharacter)
	{
		Speed = OwnerCharacter->GetVelocity().Length();

		// Offset Yaw For Strafing
		FRotator ControlRotation = OwnerCharacter->GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(OwnerCharacter->GetVelocity());
		FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, ControlRotation);
		DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaSeconds, 5.f);
		YawOffset = DeltaRotation.Yaw;

		PlayerRotationLastFrame = PlayerRotation;
		PlayerRotation = OwnerCharacter->GetActorRotation();
		const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(PlayerRotation, PlayerRotationLastFrame);
		const float Target = Delta.Yaw / DeltaSeconds;
		const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 8.f);
		Lean = FMath::Clamp(Interp, -90.f, 90.f);

		// Only rotate with camera when moving
		if (!bIsAiming)
		{
			if (Speed > 5.f)  // Moving
			{
				OwnerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
				OwnerCharacter->bUseControllerRotationYaw = true;
			}
			else  // Stationary
			{
				OwnerCharacter->GetCharacterMovement()->bOrientRotationToMovement = true;
				OwnerCharacter->bUseControllerRotationYaw = false;
			}
		}


		//FRotator BodyRotation = OwnerCharacter->GetActorRotation();
		//FRotator BodyRotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(BodyRotation, BodyPreviousRotation);
		//BodyPreviousRotation = BodyRotation;

		//YawSpeed = BodyRotationDelta.Yaw / DeltaSeconds;
		//SmoothedYawSpeed = UKismetMathLibrary::FInterpTo(SmoothedYawSpeed, YawSpeed, DeltaSeconds, YawSpeedSmoothLerpSpeed);

		LookRotationOffset = UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, PlayerRotation);

		ControlRotation.Pitch = FRotator::NormalizeAxis(ControlRotation.Pitch);

		LookRotationOffset.Pitch = FMath::Clamp(LookRotationOffset.Pitch, -45.f, 45.f);

	}

	if(OwnerMovementComp)
	{
		bIsJumping = OwnerMovementComp->IsFalling();
		bIsInAir = OwnerMovementComp->IsFalling();
	}
}

void UChrisAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
}

bool UChrisAnimInstance::ShouldDoFullBody() const
{
	return (GetSpeed() <= 0) && !(GetIsAiming());
}

void UChrisAnimInstance::OwnerAimTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	bIsAiming = NewCount != 0;
}
