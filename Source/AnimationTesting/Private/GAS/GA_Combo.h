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

    UPROPERTY(EditDefaultsOnly, Category = "Targetting")
    float TargetSweepSphereRadius = 5.f;

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
};