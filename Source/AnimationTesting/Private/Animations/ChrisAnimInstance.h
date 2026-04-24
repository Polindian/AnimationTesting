// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagContainer.h"
#include <Character/ChrisCharacter.h>
#include "ChrisAnimInstance.generated.h"


/**
 * 
 */
UCLASS()
class UChrisAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override; 
	
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	FORCEINLINE float GetSpeed() const { return Speed; }

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	FORCEINLINE bool IsMoving() const { return Speed != 0; }

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	FORCEINLINE bool IsNotMoving() const { return Speed == 0; }

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	FORCEINLINE float GetForwardSpeed() const { return ForwardSpeed; }

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	FORCEINLINE float GetRightSpeed() const { return RightSpeed; }

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	FORCEINLINE float GetYawSpeed() const { return YawSpeed; }

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	FORCEINLINE float GetSmoothYawSpeed() const { return SmoothedYawSpeed; }

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	FORCEINLINE bool GetIsJumping() const { return bIsJumping; }

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	FORCEINLINE bool GetIsInAir() const { return bIsInAir; }

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	FORCEINLINE bool GetIsOnGround() const { return !bIsJumping; }


	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	FORCEINLINE bool GetIsAiming() const { return bIsAiming; }

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	FORCEINLINE float GetLookYawOffset() const { return LookRotationOffset.Yaw; }

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	FORCEINLINE float GetLookPitchOffset() const { return LookRotationOffset.Pitch; }


	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	bool ShouldDoFullBody() const;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float YawOffset;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Lean;



private:

	void OwnerAimTagChanged(const FGameplayTag Tag, int32 NewCount);

	UPROPERTY()
	class ACharacter* OwnerCharacter;
	UPROPERTY()
	class UCharacterMovementComponent* OwnerMovementComp;

	float Speed;
	float ForwardSpeed = 0.f;
	float RightSpeed = 0.f;

	float YawSpeed;
	float SmoothedYawSpeed;



	UPROPERTY(EditAnywhere, Category = "Animation")
	float YawSpeedSmoothLerpSpeed = 1.f;
	bool bIsJumping;
	bool bIsInAir;
	bool bIsAiming;

	FRotator PlayerRotationLastFrame;
	FRotator PlayerRotation;
	FRotator DeltaRotation;
	FRotator LookRotationOffset;
};
