// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GAS/ChrisGameplayAbility.h"
#include "GAP_Dead.generated.h"

UCLASS()
class UGAP_Dead : public UChrisGameplayAbility
{
    GENERATED_BODY()
public:
    UGAP_Dead();
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "Rewards")
    float BaseExperenceReward = 100.f;

    UPROPERTY(EditDefaultsOnly, Category = "Rewards")
    float BaseSoulReward = 10.f;

    UPROPERTY(EditDefaultsOnly, Category = "Rewards")
    float ExperienceRewardPerExperience = 0.05f;

    UPROPERTY(EditDefaultsOnly, Category = "Rewards")
    float SoulRewardPerExperience = 20.f;

    UPROPERTY(EditDefaultsOnly, Category = "Rewards")
    TSubclassOf<UGameplayEffect> RewardEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Rewards")
    float HeroExperienceReward = 300.f;

    UPROPERTY(EditDefaultsOnly, Category = "Rewards")
    float HeroSoulReward = 30.f;

    // === CHANGED === Radius for finding friendly allies to share rewards with (centered on Killer)
    UPROPERTY(EditDefaultsOnly, Category = "Rewards")
    float AllyShareRadius = 400.f;

    // Finds friendly allies (same team) near the Killer, excluding the Killer
    TArray<AActor*> GetFriendlyAlliesNearKiller(AActor* Killer) const;
};