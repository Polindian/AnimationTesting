// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GenericTeamAgentInterface.h"
#include "ChrisGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class UChrisGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UChrisGameplayAbility();
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, 
		const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const override;
protected:
	class UAnimInstance* GetOwnerAnimInstance() const;
	TArray<FHitResult> GetHitResultFromSweepLocationTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle, float SphereSweepRadius = 10.f, 
		ETeamAttitude::Type TargetTeam = ETeamAttitude::Hostile, bool bDrawDebug = false, bool bIgnoreSelf = true) const;

	UFUNCTION()
	FORCEINLINE bool ShouldDrawDebug() const { return bShouldDrawDebug; }

	void PushSelf(const FVector& PushVelocity);
	void PushTarget(AActor* Target, const FVector& PushVelocity);
	ACharacter* GetOwningAvatarCharacter();

	void PushTargetsFromOwnerLocation(const FGameplayAbilityTargetDataHandle& TargetDataHandle, float PushSpeed);
	void PushTargetsFromOwnerLocation(const TArray<AActor*>& Targets, float PushSpeed);
	void PushTargetsFromLocation(const TArray<AActor*>& Targets, const FVector& Location, float PushSpeed);

	FGenericTeamId GetOwnerTeamId() const;

	void ApplyGameplayEffectToHitResultActor(const FHitResult& HitResult, TSubclassOf<UGameplayEffect> GameplayEffect, int Level = 1);
private:
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bShouldDrawDebug = true;

	UPROPERTY()
	class ACharacter* AvatarCharacter;
};
