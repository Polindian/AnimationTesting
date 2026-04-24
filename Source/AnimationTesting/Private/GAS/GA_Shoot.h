// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GAS/ChrisGameplayAbility.h"
#include "GA_Shoot.generated.h"

/**
 * 
 */
UCLASS()
class UGA_Shoot : public UChrisGameplayAbility
{
	GENERATED_BODY()
	
public:

	UGA_Shoot();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	// Override so cooldown tag doesn't block ability activation (aiming always works)
	virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void GetCooldownTimeRemainingAndDuration(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, float& TimeRemaining, float& CooldownDuration) const override;

private:
	// Montage for the transition INTO aiming (arm goes up)
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* IdleToAimMontage;

	// Montage for the transition OUT OF aiming (arm goes down)
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* AimToIdleMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* ShootMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Shoot")
	TSubclassOf<class AProjectileActor> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Shoot")
	float ShootProjectileSpeed = 3000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Shoot")
	float ShootProjectileRange = 5000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Shoot")
	TSubclassOf<UGameplayEffect> ProjectileHitEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Shoot")
	int32 MaxShots = 3;

	UPROPERTY(EditDefaultsOnly, Category = "Shoot")
	float ShootCooldownDuration = 10.f;  // Match your GE_Shoot_Cooldown duration

	int32 CurrentShotCount = 0;
	double CooldownEndTime = 0.0;
	bool IsCooldownActive() const;
	FTimerHandle ShootCooldownTimerHandle;
	void OnShootCooldownFinished();
	
	static FGameplayTag GetShootTag();

	UFUNCTION()
	void StopShooting(FGameplayEventData Payload);
	UFUNCTION()
	void ShootProjectile(FGameplayEventData Payload);
	
};
