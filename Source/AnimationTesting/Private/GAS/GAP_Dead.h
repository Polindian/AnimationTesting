// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GAS/ChrisGameplayAbility.h"
#include "GAP_Dead.generated.h"

/**
 * 
 */
UCLASS()
class UGAP_Dead : public UChrisGameplayAbility
{
	GENERATED_BODY()
public:
	UGAP_Dead();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Rewards")
	float RewardRange = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Rewards")
	float BaseExperenceReward = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Rewards")
	float BaseSoulReward = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Rewards")
	float ExperienceRewardPerExperience = 0.05f;

	UPROPERTY(EditDefaultsOnly, Category = "Rewards")
	float SoulRewardPerExperience = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Rewards")
	float KillerRewardPortion = 0.5f;

	TArray<AActor*> GetRewardTargets() const;

	UPROPERTY(EditDefaultsOnly, Category = "Rewards")
	TSubclassOf<UGameplayEffect> RewardEffect;
};
