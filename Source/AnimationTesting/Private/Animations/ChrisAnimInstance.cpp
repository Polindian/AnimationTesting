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
    if (!OwnerCharacter || DeltaSeconds <= 0.f) return;

    // --- Speed (2D only, ignore Z) ---
    FVector Velocity = OwnerCharacter->GetVelocity();
    Speed = Velocity.Size2D();

    // --- Yaw Offset & Speed ---
    FRotator ControlRotation = OwnerCharacter->GetBaseAimRotation();
    FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(Velocity);
    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, ControlRotation);

    // 1. Calculate raw yaw
    if (Speed > 100.f)
    {
        DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaSeconds, 15.f);
    }

    float RawYaw = DeltaRotation.Yaw;

    if (Speed > 10.f)
    {
        // Don't override speed if an ability is controlling it
        UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerCharacter);
        if (ASC && !ASC->HasMatchingGameplayTag(UChrisAbilitySystemStatics::GetSpeedOverrideTag()))
        {
            float AbsYaw = FMath::Abs(RawYaw);

            float TargetSpeed;
            if (AbsYaw <= 67.5f)
                TargetSpeed = ForwardMoveSpeed;
            else if (AbsYaw <= 112.5f)
                TargetSpeed = StrafeMoveSpeed;
            else if (AbsYaw <= 157.5f)
                TargetSpeed = BackDiagonalMoveSpeed;
            else
                TargetSpeed = BackwardMoveSpeed;

            OwnerMovementComp->MaxWalkSpeed = FMath::FInterpTo(
                OwnerMovementComp->MaxWalkSpeed,
                TargetSpeed,
                DeltaSeconds,
                8.f
            );
        }
    }

    // 3. Quantize yaw for blendspace (smooth)
  
    float QuantizedYaw = FMath::RoundToFloat(RawYaw / 45.f) * 45.f;

    // Wrap-safe interpolation: find the shortest angular path
    float YawDelta = FRotator::NormalizeAxis(QuantizedYaw - YawOffset);
    YawOffset = FRotator::NormalizeAxis(YawOffset + YawDelta * FMath::Clamp(DeltaSeconds * 10.f, 0.f, 1.f));

  
    /**/
    // --- Lean (keep for cosmetic use, but NOT as a blendspace axis) ---
    PlayerRotationLastFrame = PlayerRotation;
    PlayerRotation = OwnerCharacter->GetActorRotation();
    const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(PlayerRotation, PlayerRotationLastFrame);
    float ClampedYawDelta = FMath::Clamp(Delta.Yaw, -10.f, 10.f);
    const float Target = ClampedYawDelta / DeltaSeconds;
    const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f);
    Lean = FMath::Clamp(Interp, -45.f, 45.f); // Tighter range since it's cosmetic now

    // --- Rotation Mode ---
    if (!bIsAiming)
    {
        if (Speed > 10.f)
        {
            OwnerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
            OwnerCharacter->bUseControllerRotationYaw = false;
            OwnerCharacter->GetCharacterMovement()->bUseControllerDesiredRotation = true;
        }
        else
        {
            OwnerCharacter->GetCharacterMovement()->bOrientRotationToMovement = true;
            OwnerCharacter->bUseControllerRotationYaw = false;
            OwnerCharacter->GetCharacterMovement()->bUseControllerDesiredRotation = false;
        }
    }

    // --- Look Offset ---
    LookRotationOffset = UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, PlayerRotation);
    LookRotationOffset.Pitch = FMath::Clamp(FRotator::NormalizeAxis(ControlRotation.Pitch), -45.f, 45.f);

    // --- Jump ---
    if (OwnerMovementComp)
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
