// Christopher Naglik All Rights Reserved


#include "GAS/GA_Combo.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagsManager.h"
#include "GAS/ChrisAbilitySystemStatics.h"


UGA_Combo::UGA_Combo()
{
    AbilityTags.AddTag(UChrisAbilitySystemStatics::GetBasicAttackAbilityTag());
    BlockAbilitiesWithTag.AddTag(UChrisAbilitySystemStatics::GetBasicAttackAbilityTag());
    ActivationRequiredTags.AddTag(UChrisAbilitySystemStatics::GetWeaponsEquippedTag());
}

void UGA_Combo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!K2_CommitAbility())
    {
        K2_EndAbility();
        return;
    }

    if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
    {
        UAbilityTask_PlayMontageAndWait* PlayComboMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ComboMontage);
        PlayComboMontageTask->OnBlendOut.AddDynamic(this, &UGA_Combo::K2_EndAbility);
        PlayComboMontageTask->OnCompleted.AddDynamic(this, &UGA_Combo::K2_EndAbility);
        PlayComboMontageTask->OnInterrupted.AddDynamic(this, &UGA_Combo::K2_EndAbility);
        PlayComboMontageTask->OnCancelled.AddDynamic(this, &UGA_Combo::K2_EndAbility);
        PlayComboMontageTask->ReadyForActivation();

        UAbilityTask_WaitGameplayEvent* WaitForComboChangeEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetComboChangedEventTag(), nullptr, false, false);
        WaitForComboChangeEventTask->EventReceived.AddDynamic(this, &UGA_Combo::ComboChangedEventReceived);
        WaitForComboChangeEventTask->ReadyForActivation();
    }

    if (K2_HasAuthority())
    {
        UAbilityTask_WaitGameplayEvent* WaitForTargetEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetComboTargetEventTag());
        WaitForTargetEventTask->EventReceived.AddDynamic(this, &UGA_Combo::DoDamage);
        WaitForTargetEventTask->ReadyForActivation();
    }

    // === CHANGED === was: SetupWaitComboInputPress();
    SetupWaitComboCommitEvent();
}

FGameplayTag UGA_Combo::GetComboChangedEventTag()
{
    return FGameplayTag::RequestGameplayTag("ability.combo.change");
}

FGameplayTag UGA_Combo::GetComboChangedEventEndTag()
{
    return FGameplayTag::RequestGameplayTag("ability.combo.change.end");
}

FGameplayTag UGA_Combo::GetComboTargetEventTag()
{
    return FGameplayTag::RequestGameplayTag("ability.combo.damage");
}

// === CHANGED === was: SetupWaitComboInputPress() using WaitInputPress
// Now mirrors StabAbility lines 73-75 exactly
void UGA_Combo::SetupWaitComboCommitEvent()
{
    UAbilityTask_WaitGameplayEvent* WaitComboCommitEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, UChrisAbilitySystemStatics::GetBasicAttack1InputPressedTag());
    WaitComboCommitEvent->EventReceived.AddDynamic(this, &UGA_Combo::HandleComboCommitLeftSlashEvent);
    WaitComboCommitEvent->ReadyForActivation();
        
    // === NEW === also listen for right hand input (for third combo section)
    UAbilityTask_WaitGameplayEvent* WaitComboCommitEvent2 = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, UChrisAbilitySystemStatics::GetBasicAttackInputPressedTag());
    WaitComboCommitEvent2->EventReceived.AddDynamic(this, &UGA_Combo::HandleComboCommitRightSlashEvent);
    WaitComboCommitEvent2->ReadyForActivation();
}

void UGA_Combo::HandleComboCommitLeftSlashEvent(FGameplayEventData EventData)
{
    // === CHANGED === was: TryCommitNextCombo() with no check
    // Now only allows left hand input to advance from the first section
    UAnimInstance* OwnerAnimInstance = GetOwnerAnimInstance();
    if (!OwnerAnimInstance) return;
    FName CurrentSection = OwnerAnimInstance->Montage_GetCurrentSection(ComboMontage);

    // RightSlash (true):  left input advances combo01
     // LeftSlash  (false): left input advances combo02
    if (bStartsWithRightHand && CurrentSection == FName("combo01"))
    {
        TryCommitNextCombo();
    }
    else if (!bStartsWithRightHand && CurrentSection == FName("combo02"))
    {
        TryCommitNextCombo();
    }
}

// === NEW === Only advances if currently on combo02 (second section -> third)
void UGA_Combo::HandleComboCommitRightSlashEvent(FGameplayEventData EventData)
{
    UAnimInstance* OwnerAnimInstance = GetOwnerAnimInstance();
    if (!OwnerAnimInstance) return;
    FName CurrentSection = OwnerAnimInstance->Montage_GetCurrentSection(ComboMontage);

    // RightSlash (true):  right input advances combo02
     // LeftSlash  (false): right input advances combo01
    if (bStartsWithRightHand && CurrentSection == FName("combo02"))
    {
        TryCommitNextCombo();
    }
    else if (!bStartsWithRightHand && CurrentSection == FName("combo01"))
    {
        TryCommitNextCombo();
    }
}

void UGA_Combo::TryCommitNextCombo()
{
    if (NextComboName == NAME_None)
    {
        return;
    }

    UAnimInstance* OwnerAnimInstance = GetOwnerAnimInstance();
    if (!OwnerAnimInstance)
    {
        return;
    }
    OwnerAnimInstance->Montage_SetNextSection(OwnerAnimInstance->Montage_GetCurrentSection(ComboMontage), NextComboName, ComboMontage);
}





TSubclassOf<UGameplayEffect> UGA_Combo::GetDamageEffectForCurrentCombo() const
{
    UAnimInstance* OwnerAnimInstance = GetOwnerAnimInstance();
    if (OwnerAnimInstance)
    {
        FName CurrentSectionName = OwnerAnimInstance->Montage_GetCurrentSection(ComboMontage);
        const TSubclassOf<UGameplayEffect>* FoundEffectPtr = DamageEffectMap.Find(CurrentSectionName);
        if (FoundEffectPtr)
        {
            return *FoundEffectPtr;
        }
    }

    return DefaultDamageEffect;
}

void UGA_Combo::ComboChangedEventReceived(FGameplayEventData Data)
{
    FGameplayTag EventTag = Data.EventTag;

    if (EventTag == GetComboChangedEventEndTag())
    {
        NextComboName = NAME_None;
        return;
    }

    TArray<FName> TagNames;
    UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag, TagNames);
    NextComboName = TagNames.Last();
}

void UGA_Combo::DoDamage(FGameplayEventData Data)
{
    TArray<FHitResult> HitResults = GetHitResultFromSweepLocationTargetData(Data.TargetData, TargetSweepSphereRadius);

    for (const FHitResult& HitResult : HitResults)
    {
        TSubclassOf<UGameplayEffect> GameplayEffect = GetDamageEffectForCurrentCombo();
        ApplyGameplayEffectToHitResultActor(HitResult, GameplayEffect, GetAbilityLevel());
    }
}