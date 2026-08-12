// Christopher Naglik All Rights Reserved

#include "GAS/GA_BlindBomb.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GenericTeamAgentInterface.h"

void UGA_BlindBomb::ActivateAbility(
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

    if (K2_HasAuthority())
    {
        AActor* OwnerActor = GetAvatarActorFromActorInfo();
        if (!OwnerActor || !BlindEffect)
        {
            K2_EndAbility();
            return;
        }

        // ======================================================
        // SPHERE OVERLAP: find all pawns within BlindRadius
        // ======================================================
        TArray<FOverlapResult> Overlaps;
        FCollisionShape SphereShape = FCollisionShape::MakeSphere(BlindRadius);
        FCollisionObjectQueryParams ObjectParams;
        ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

        GetWorld()->OverlapMultiByObjectType(
            Overlaps,
            OwnerActor->GetActorLocation(),
            FQuat::Identity,
            ObjectParams,
            SphereShape);

        // ======================================================
        // TEAM CHECK: for each overlapping pawn, verify it's
        // hostile before applying the blind. 
        // ======================================================
        IGenericTeamAgentInterface* OwnerTeam = Cast<IGenericTeamAgentInterface>(OwnerActor);
        UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo();

        FGameplayEffectSpecHandle BlindSpec = MakeOutgoingGameplayEffectSpec(BlindEffect, 1);

        if (BlindSpec.IsValid() && OwnerASC)
        {
            // Track actors we've already blinded to avoid duplicates
            TSet<AActor*> BlindedActors;

            for (const FOverlapResult& Overlap : Overlaps)
            {
                AActor* Target = Overlap.GetActor();

                if (!Target || Target == OwnerActor || BlindedActors.Contains(Target))
                {
                    continue;
                }

                if (OwnerTeam && OwnerTeam->GetTeamAttitudeTowards(*Target) != ETeamAttitude::Hostile)
                {
                    continue;
                }

                UAbilitySystemComponent* TargetASC =
                    UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);

                if (TargetASC)
                {
                    OwnerASC->ApplyGameplayEffectSpecToTarget(
                        *BlindSpec.Data.Get(), TargetASC);

                    BlindedActors.Add(Target);

                    UE_LOG(LogTemp, Warning, TEXT("[BlindBomb] Blinded: %s"),
                        *Target->GetName());
                }
            }

            UE_LOG(LogTemp, Warning, TEXT("[BlindBomb] Blinded %d enemies within %.0f units"),
                BlindedActors.Num(), BlindRadius);
        }
    }

    K2_EndAbility();
}