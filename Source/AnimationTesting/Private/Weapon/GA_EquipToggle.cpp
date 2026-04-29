// Christopher Naglik All Rights Reserved


#include "Weapon/GA_EquipToggle.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GAS/ChrisAbilitySystemStatics.h"
#include "Weapon/SwordEquipComponent.h"

UGA_EquipToggle::UGA_EquipToggle()
{
    // This ability should NOT require weapons.equipped — it works in both states.
    // It still blocks on stun/dead from the base class constructor.

    // Only one equip/unequip at a time
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    ActivationBlockedTags.AddTag(UChrisAbilitySystemStatics::GetAimStatsTag());
}


void UGA_EquipToggle::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!K2_CommitAbility())
    {
        K2_EndAbility();
        return;
    }

    // Find the SwordEquipComponent on our avatar actor
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor)
    {
        K2_EndAbility();
        return;
    }

    USwordEquipComponent* SwordComp = AvatarActor->FindComponentByClass<USwordEquipComponent>();
    if (!SwordComp)
    {
        K2_EndAbility();
        return;
    }

    // Decide which montage to play based on current state
    UAnimMontage* MontageToPlay = nullptr;
    ESwordEquipState CurrentState = SwordComp->GetEquipState();

    if (CurrentState == ESwordEquipState::Unequipped)
    {
        MontageToPlay = EquipMontage;
        SwordComp->BeginEquip();
    }
    else if (CurrentState == ESwordEquipState::Equipped)
    {
        MontageToPlay = UnequipMontage;
        SwordComp->BeginUnequip();
    }
    else
    {
        // Already transitioning — don't do anything
        K2_EndAbility();
        return;
    }

    if (!MontageToPlay)
    {
        K2_EndAbility();
        return;
    }

    // Play montage on upper body slot
    UAbilityTask_PlayMontageAndWait* PlayMontage =
        UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, NAME_None, MontageToPlay);

    PlayMontage->OnBlendOut.AddDynamic(this, &UGA_EquipToggle::K2_EndAbility);
    PlayMontage->OnCancelled.AddDynamic(this, &UGA_EquipToggle::K2_EndAbility);
    PlayMontage->OnInterrupted.AddDynamic(this, &UGA_EquipToggle::K2_EndAbility);
    PlayMontage->OnCompleted.AddDynamic(this, &UGA_EquipToggle::K2_EndAbility);
    PlayMontage->ReadyForActivation();
}


void UGA_EquipToggle::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}