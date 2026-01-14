// Christopher Naglik All Rights Reserved


#include "Animations/ChrisAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UChrisAnimInstance::NativeInitializeAnimation()
{
	OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
	if(OwnerCharacter)
	{
		OwnerMovementComp = OwnerCharacter->GetCharacterMovement();
	}
}

void UChrisAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if (OwnerCharacter)
	{
		Speed = OwnerCharacter->GetVelocity().Length();

		FRotator BodyRotation = OwnerCharacter->GetActorRotation();
		FRotator BodyRotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(BodyRotation, BodyPreviousRotation);
		BodyPreviousRotation = BodyRotation;

		YawSpeed = BodyRotationDelta.Yaw / DeltaSeconds;
		SmoothedYawSpeed = UKismetMathLibrary::FInterpTo(SmoothedYawSpeed, YawSpeed, DeltaSeconds, YawSpeedSmoothLerpSpeed);

		FRotator ControlRotation = OwnerCharacter->GetBaseAimRotation();
		LookRotationOffset = UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, BodyRotation);

		AChrisCharacter* ChrisChar = Cast<AChrisCharacter>(OwnerCharacter);
		if (ChrisChar)
		{
			CurrentGait = ChrisChar->GetCurrentGait();
		}
	}

	if(OwnerMovementComp)
	{
		bIsJumping = OwnerMovementComp->IsFalling();
		bIsCrouching = OwnerMovementComp->IsCrouching();
	}
}

void UChrisAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
}
