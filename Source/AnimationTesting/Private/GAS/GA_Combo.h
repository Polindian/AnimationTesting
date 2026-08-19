// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GAS/ChrisGameplayAbility.h"
#include "GA_Combo.generated.h"

/**
 * 
 */
UCLASS()
class UGA_Combo : public UChrisGameplayAbility
{
    GENERATED_BODY()

public:
    UGA_Combo();
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    static FGameplayTag GetComboChangedEventTag();
    static FGameplayTag GetComboChangedEventEndTag();
    static FGameplayTag GetComboTargetEventTag();

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,bool bReplicateEndAbility, bool bWasCancelled) override;

private:
    // === CHANGED === was: void SetupWaitComboInputPress();
    void SetupWaitComboCommitEvent();

    // === CHANGED === was: void HandleInputPress(float TimeWaited);
    // Now mirrors StabAbility::HandleComboCommitEvent exactly
    UFUNCTION()
    void HandleComboCommitLeftSlashEvent(FGameplayEventData EventData);

    // === NEW === separate callback for right hand input (combo03)
    UFUNCTION()
    void HandleComboCommitRightSlashEvent(FGameplayEventData EventData);

    void TryCommitNextCombo();

    // === NEW === set to true for RightSlash BP, false for LeftSlash BP
    UPROPERTY(EditDefaultsOnly, Category = "Combo")
    bool bStartsWithRightHand = true;

    UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effect")
    TSubclassOf<UGameplayEffect> DefaultDamageEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effect")
    TMap<FName, TSubclassOf<UGameplayEffect>> DamageEffectMap;

    TSubclassOf<UGameplayEffect> GetDamageEffectForCurrentCombo() const;

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* ComboMontage;

    UFUNCTION()
    void ComboChangedEventReceived(FGameplayEventData Data);

    UFUNCTION()
    void DoDamage(FGameplayEventData Data);

    FName NextComboName;

    // Speed Override

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float AttackMoveSpeed = 50.f;


    UPROPERTY(EditDefaultsOnly, Category = "VFX")
    TSubclassOf<UGameplayEffect> ComboFlashEffect;
};