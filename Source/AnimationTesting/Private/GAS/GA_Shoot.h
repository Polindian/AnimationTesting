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

	/** Clears the volley state at a round boundary. The GAS cooldown reset can't
	 *  reach this ability because its cooldown is tracked internally, not by a GE. */
	void ResetShootState();

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	// Override so cooldown tag doesn't block ability activation (aiming always works)
	virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;



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

	// Alternate projectile used when the shooter has the Deadeye upgrade.
	// Spawned server-side, so its class replicates to every client.
	UPROPERTY(EditDefaultsOnly, Category = "Shoot")
	TSubclassOf<class AProjectileActor> DeadeyeProjectileClass;

	// Hit effect used when Deadeye is active — carries its own impact cue.
	UPROPERTY(EditDefaultsOnly, Category = "Shoot")
	TSubclassOf<UGameplayEffect> DeadeyeProjectileHitEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Shoot")
	float ShootProjectileSpeed = 3000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Shoot")
	float ShootProjectileRange = 5000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Shoot")
	TSubclassOf<UGameplayEffect> ProjectileHitEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Shoot")
	int32 MaxShots = 3;

	UPROPERTY(EditDefaultsOnly, Category = "Shoot")
	float ShootCooldownDuration = 100.f; 

	
	float GetLeveledCooldownDuration() const;

	int32 CurrentShotCount = 0;

	bool IsShootOnCooldown() const;

	FTimerHandle ShootCooldownTimerHandle;
	void OnShootCooldownFinished();
	double LastShotTime = 0.0;
	
	static FGameplayTag GetShootTag();

	UFUNCTION()
	void StopShooting(FGameplayEventData Payload);
	UFUNCTION()
	void ShootProjectile(FGameplayEventData Payload);

	bool bEventTasksCreated = false;

	bool IsDeadeyeActive() const;
	
};
