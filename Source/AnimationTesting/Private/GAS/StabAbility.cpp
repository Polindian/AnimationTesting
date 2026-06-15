// Christopher Naglik All Rights Reserved


#include "GAS/StabAbility.h"
#include "GAS/GA_Combo.h"
#include "GAS/ChrisAbilitySystemStatics.h"
#include "GAS/ChrisAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameplayTagsManager.h"


UStabAbility::UStabAbility()
{
    ActivationRequiredTags.AddTag(UChrisAbilitySystemStatics::GetWeaponsEquippedTag());
}

void UStabAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!K2_CommitAbility())
    {
        K2_EndAbility();
        return;
    }

    if (ACharacter* OwnerChar = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
    {
        OwnerChar->GetCharacterMovement()->MaxWalkSpeed = AttackMoveSpeed;
    }

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (ASC)
    {
        ASC->AddLooseGameplayTag(UChrisAbilitySystemStatics::GetSpeedOverrideTag());
    }

    // ======================================================
    // RADIUS GE APPLICATION
    //
    // If a RadiusSweepEffect is assigned (smash/stun), apply it to self.
    // We use GetAbilityLevel() to pass the current level into the GE spec,
    // which makes the scalable float evaluate the curve table at that level.
    // ======================================================
    if (RadiusSweepEffect)
    {
        FGameplayEffectSpecHandle RadiusSpec = MakeOutgoingGameplayEffectSpec(
            RadiusSweepEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));

        if (RadiusSpec.IsValid())
        {
            ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, RadiusSpec);
        }
    }

    if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
    {
        UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, RightStabMontage);
        PlayMontageTask->OnBlendOut.AddDynamic(this, &UStabAbility::K2_EndAbility);
        PlayMontageTask->OnCancelled.AddDynamic(this, &UStabAbility::K2_EndAbility);
        PlayMontageTask->OnInterrupted.AddDynamic(this, &UStabAbility::K2_EndAbility);
        PlayMontageTask->OnCompleted.AddDynamic(this, &UStabAbility::K2_EndAbility);
        PlayMontageTask->ReadyForActivation();

        UAbilityTask_WaitGameplayEvent* WaitForEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetRightStabTag());
        WaitForEventTask->EventReceived.AddDynamic(this, &UStabAbility::StartCombo);
        WaitForEventTask->ReadyForActivation();
    }
    NextComboName = NAME_None;
}

void UStabAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    // AnimInstance takes back control
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (ASC)
    {
        ASC->RemoveLooseGameplayTag(UChrisAbilitySystemStatics::GetSpeedOverrideTag());
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FGameplayTag UStabAbility::GetRightStabTag()
{
    return FGameplayTag::RequestGameplayTag("ability.stab.right");
}

const FGenericDamageEffectDefinition* UStabAbility::GetDamageEffectDefinitionForCurrentCombo() const
{
    UAnimInstance* OwnerAnimInstance = GetOwnerAnimInstance();
    if (OwnerAnimInstance)
    {
        FName CurrentComboName = OwnerAnimInstance->Montage_GetCurrentSection(RightStabMontage);
        const FGenericDamageEffectDefinition* EffectDefinition = ComboDamageMap.Find(CurrentComboName);
        return EffectDefinition;
    }
    return nullptr;
}

void UStabAbility::StartCombo(FGameplayEventData EventData)
{
    // Freeze movement on ground impact for smash / stun
        if (RadiusAttribute.IsValid())
        {
            if (ACharacter* OwnerChar = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
            {
                OwnerChar->GetCharacterMovement()->MaxWalkSpeed = 15.f;
            }
        }


    if (K2_HasAuthority())
    {
        int HitResultCount = UAbilitySystemBlueprintLibrary::GetDataCountFromTargetData(EventData.TargetData);

        for (int i = 0; i < HitResultCount; i++)
        {
            FHitResult HitResult = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(EventData.TargetData, i);

            // Skip airborne targets for AoE ground attacks (smash/stun)
            if (RadiusAttribute.IsValid())
            {
                ACharacter* TargetCharacter = Cast<ACharacter>(HitResult.GetActor());
                if (TargetCharacter && TargetCharacter->GetCharacterMovement()->IsFalling())
                {
                    continue;
                }
            }

            // Apply Push Velocity
            if (!InitialHitPushVelocity.IsNearlyZero())
            {
                FVector PushVelocity = GetAvatarActorFromActorInfo()->GetActorTransform().TransformVector(InitialHitPushVelocity);
                PushTarget(HitResult.GetActor(), PushVelocity);
            }

            ApplyGameplayEffectToHitResultActor(HitResult, StabDamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
        }
    }

    UAbilityTask_WaitGameplayEvent* WaitComboChangeEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, UGA_Combo::GetComboChangedEventTag(), nullptr, false, false);
    WaitComboChangeEvent->EventReceived.AddDynamic(this, &UStabAbility::HandleComboChangeEvent);
    WaitComboChangeEvent->ReadyForActivation();

    // Collect unique input tags from ComboInputMap and register listeners
    TSet<FGameplayTag> UniqueInputTags;
    for (const auto& Pair : ComboInputMap)
    {
        if (Pair.Value.IsValid())
        {
            UniqueInputTags.Add(Pair.Value);
        }
    }

    for (const FGameplayTag& InputTag : UniqueInputTags)
    {
        UAbilityTask_WaitGameplayEvent* WaitComboCommitEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, InputTag);
        WaitComboCommitEvent->EventReceived.AddDynamic(this, &UStabAbility::HandleComboCommit);
        WaitComboCommitEvent->ReadyForActivation();
    }

    UAbilityTask_WaitGameplayEvent* WaitComboDamageEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, UGA_Combo::GetComboTargetEventTag());
    WaitComboDamageEvent->EventReceived.AddDynamic(this, &UStabAbility::HandleComboDamageEvent);
    WaitComboDamageEvent->ReadyForActivation();
}


/**********************************************/
/*                Combo Changes               */
/**********************************************/

void UStabAbility::HandleComboChangeEvent(FGameplayEventData EventData)
{
    FGameplayTag EventTag = EventData.EventTag;
    if (EventTag == UGA_Combo::GetComboChangedEventEndTag())
    {
        NextComboName = NAME_None;
        return;
    }
    TArray<FName> TagNames;
    UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag, TagNames);

    NextComboName = TagNames.Last();
}

void UStabAbility::HandleComboCommit(FGameplayEventData EventData)
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


    // Get current section and check if the received input tag matches what's required
    FName CurrentSection = OwnerAnimInstance->Montage_GetCurrentSection(RightStabMontage);
    const FGameplayTag* RequiredInputTag = ComboInputMap.Find(CurrentSection);

    // Only advance if this is the correct input for the current section
    if (RequiredInputTag && *RequiredInputTag == EventData.EventTag)
    {
        OwnerAnimInstance->Montage_SetNextSection(CurrentSection, NextComboName, RightStabMontage);
    }
}

void UStabAbility::HandleComboDamageEvent(FGameplayEventData EventData)
{
    if (K2_HasAuthority())
    {
        // ======================================================
        // DETERMINE THE SWEEP RADIUS
        //
        // Two modes of operation:
        //
        // MODE A (AoE abilities like Smash/Stun):
        //   - RadiusAttribute is configured (points to SmashSweepRadius or StunSweepRadius)
        //   - We read the attribute from the owner's ASC
        //   - The value was set earlier in ActivateAbility via the RadiusSweepEffect GE
        //   - We then do a sphere sweep at the location from the anim notify
        //     and use THOSE results for damage (AoE detection)
        //
        // MODE B (Light attacks / regular stab):
        //   - RadiusAttribute is NOT configured (left empty in Blueprint)
        //   - We use the fixed TargetSweepRadius for the sweep
        //   - Damage is applied to the raw EventData.TargetData from the
        //     socket trail (an_sendtargetgroup), because the socket trail
        //     already provides accurate per-hit detection along the weapon path
        // ======================================================
        float EvaluatedRadius = TargetSweepRadius;
        bool bUseAoESweepResults = false;

        if (RadiusAttribute.IsValid())
        {
            // AoE mode: read the radius from our attribute (set by curve table GE)
            UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo();
            if (OwnerASC)
            {
                bool bFound = false;
                float AttrRadius = OwnerASC->GetGameplayAttributeValue(RadiusAttribute, bFound);

                if (bFound && AttrRadius > 0.f)
                {
                    EvaluatedRadius = AttrRadius;
                    bUseAoESweepResults = true;
                }
            }
        }

        // Perform the sphere sweep at the location(s) provided by the anim notify.
        // Pass ShouldDrawDebug() so the debug sphere actually renders in-game.
        TArray<FHitResult> TargetHitResults = GetHitResultFromSweepLocationTargetData(
            EventData.TargetData, EvaluatedRadius, ETeamAttitude::Hostile, ShouldDrawDebug());

        const FGenericDamageEffectDefinition* EffectDefinition = GetDamageEffectDefinitionForCurrentCombo();
        if (!EffectDefinition)
        {
            return;
        }

        if (bUseAoESweepResults)
        {
            // ======================================================
            // MODE A: AoE abilities (Smash/Stun)
            // ======================================================
            UE_LOG(LogTemp, Warning, TEXT("SMASH: Using Mode A (AoE sweep)"));
            
            for (const FHitResult& HitResult : TargetHitResults)
            {
                // Skip airborne targets — they dodge AoE ground attacks
                ACharacter* TargetCharacter = Cast<ACharacter>(HitResult.GetActor());
                
                if (TargetCharacter && !TargetCharacter->GetCharacterMovement()->IsFalling())
                {
                    continue;
                }
                
                FVector PushVelocity = GetAvatarActorFromActorInfo()->GetActorTransform().TransformVector(EffectDefinition->PushVelocity);
                PushTarget(HitResult.GetActor(), PushVelocity);
                ApplyGameplayEffectToHitResultActor(HitResult, EffectDefinition->DamageEffect,
                    GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
            }
        }
        else
        {
            // ======================================================
            // MODE B: Light attacks (regular stab combos)
            // ======================================================
            UE_LOG(LogTemp, Warning, TEXT("SMASH: Using Mode B (socket trail)"));
            int HitResultCount = UAbilitySystemBlueprintLibrary::GetDataCountFromTargetData(EventData.TargetData);

            for (int i = 0; i < HitResultCount; i++)
            {
                FHitResult HitResult = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(EventData.TargetData, i);
                FVector PushVelocity = GetAvatarActorFromActorInfo()->GetActorTransform().TransformVector(EffectDefinition->PushVelocity);
                PushTarget(HitResult.GetActor(), PushVelocity);
                ApplyGameplayEffectToHitResultActor(HitResult, EffectDefinition->DamageEffect,
                    GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
            }
        }
    }
}