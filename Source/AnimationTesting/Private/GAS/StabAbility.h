// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GAS/ChrisGameplayAbility.h"
#include "GAS/ChrisGameplayAbilityTypes.h"
#include "GameplayTagContainer.h"
#include "StabAbility.generated.h"

/**
 *
 */
UCLASS()
class UStabAbility : public UChrisGameplayAbility
{
    GENERATED_BODY()
public:
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

    UStabAbility();
private:
    UPROPERTY(EditDefaultsOnly, Category = "Stab")
    TSubclassOf<UGameplayEffect> StabDamageEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Combo")
    TMap<FName, FGenericDamageEffectDefinition> ComboDamageMap;

    UPROPERTY(EditDefaultsOnly, Category = "Launch")
    float HitLaunchSpeed = 500.f;

    UPROPERTY(EditDefaultsOnly, Category = "Launch")
    FVector InitialHitPushVelocity;

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* RightStabMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Targetting")
    float TargetSweepRadius = 35.f;

    // ======================================================
    // RADIUS SCALING (for Smash/Stun only)
    //
    // When a RadiusSweepEffect is assigned, the ability will:
    //   1. Apply this GE to self on activation (sets the radius attribute
    //      from a curve table at the ability's current level)
    //   2. Read the attribute back in HandleComboDamageEvent to determine
    //      the actual sphere sweep radius
    //   3. Use the sphere sweep results for damage (AoE detection)
    //
    // When left empty (e.g. light attacks), the ability uses the fixed
    // TargetSweepRadius and the raw EventData from the socket trail instead.
    // ======================================================

    // GE that sets the radius attribute from a curve table (e.g. GE_SmashRadius)
    UPROPERTY(EditDefaultsOnly, Category = "Targetting")
    TSubclassOf<UGameplayEffect> RadiusSweepEffect;

    // Which attribute holds the evaluated radius (e.g. ChrisAttributeSet.SmashSweepRadius)
    UPROPERTY(EditDefaultsOnly, Category = "Targetting")
    FGameplayAttribute RadiusAttribute;

    static FGameplayTag GetRightStabTag();

    const FGenericDamageEffectDefinition* GetDamageEffectDefinitionForCurrentCombo() const;

    UFUNCTION()
    void StartCombo(FGameplayEventData EventData);

    /**********************************************/
    /*                Combo Changes               */
    /**********************************************/

    UFUNCTION()
    void HandleComboChangeEvent(FGameplayEventData EventData);

    UFUNCTION()
    void HandleComboCommit(FGameplayEventData EventData);

    UFUNCTION()
    void HandleComboDamageEvent(FGameplayEventData EventData);

    // Maps montage section name to the input tag required to advance FROM that section
    UPROPERTY(EditDefaultsOnly, Category = "Combo")
    TMap<FName, FGameplayTag> ComboInputMap;

    FName NextComboName;

    // Spped Override

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float AttackMoveSpeed = 50.f;

    FActiveGameplayEffectHandle SpeedOverrideHandle;

    UPROPERTY(EditAnywhere, Category = "Movement|Speed")
    float BlendToSpeed = 600.f;

    // ======================================================
    // CASTER AUDIO + VFX Scaling 
    //
    // Optional 2D sound layered on top of the replicated 3D
    // skill sound, heard only by the player who cast it.
    // Skill VFX become bigger depending on smash level 
    // ======================================================
    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    FGameplayTag CasterSoundTag;

    UPROPERTY(EditDefaultsOnly, Category = "VFX")
    FGameplayTag ImpactCueTag;
};