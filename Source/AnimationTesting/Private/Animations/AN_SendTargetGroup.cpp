// Christopher Naglik All Rights Reserved


#include "Animations/AN_SendTargetGroup.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GameplayCueManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameplayEffectTypes.h"

void UAN_SendTargetGroup::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp) return;

    if (TargetSocketNames.Num() <= 1) return;

    if (!MeshComp->GetOwner() || !UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner()))
    {
        return;
    }

    // ======================================================
    // DETERMINE THE SWEEP RADIUS
    // If a RadiusAttribute is configured, we read the radius from the
    // owner's Ability System Component. 
    // 
    // If RadiusAttribute is NOT set (light attacks), we fall back to
    // the fixed SphereSweepRadius property on this notify instance.
    // ======================================================
    float EvaluatedRadius = SphereSweepRadius;

    if (RadiusAttribute.IsValid())
    {
        UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner());
        if (OwnerASC)
        {
            bool bFound = false;
            float AttrRadius = OwnerASC->GetGameplayAttributeValue(RadiusAttribute, bFound);

            // Only use the attribute value if it was found and is positive.
            // Otherwise fall back to SphereSweepRadius as a safety net.
            if (bFound && AttrRadius > 0.f)
            {
                EvaluatedRadius = AttrRadius;
            }
        }
    }

    FGameplayEventData Data;
    TSet<AActor*> HitActors;
    AActor* OwnerActor = MeshComp->GetOwner();
    const IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(OwnerActor);

    for (int i = 1; i < TargetSocketNames.Num(); i++)
    {
        FVector StartLocation = MeshComp->GetSocketLocation(TargetSocketNames[i - 1]);
        FVector EndLocation = MeshComp->GetSocketLocation(TargetSocketNames[i]);

        TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
        ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));
        TArray<FHitResult> HitResults;

        TArray<AActor*> ActorsToIgnore;
        if (bIgnoreOwner)
        {
            ActorsToIgnore.Add(OwnerActor);
        }

        EDrawDebugTrace::Type DrawDebugTrace = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

        // Use EvaluatedRadius instead of the fixed SphereSweepRadius
        UKismetSystemLibrary::SphereTraceMultiForObjects(MeshComp, StartLocation, EndLocation, EvaluatedRadius, ObjectTypes, false, ActorsToIgnore, DrawDebugTrace, HitResults, false);

        for (const FHitResult& HitResult : HitResults)
        {
            if (HitActors.Contains(HitResult.GetActor()))
            {
                continue;
            }
            if (OwnerTeamInterface)
            {
                if (OwnerTeamInterface->GetTeamAttitudeTowards(*HitResult.GetActor()) != TargetTeam)
                {
                    continue;
                }

                HitActors.Add(HitResult.GetActor());

                FGameplayAbilityTargetData_SingleTargetHit* TargetHit = new FGameplayAbilityTargetData_SingleTargetHit(HitResult);
                Data.TargetData.Add(TargetHit);
                SendLocalGameplayCue(HitResult);
            }
        }

    }

    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), EventTag, Data);
}

void UAN_SendTargetGroup::SendLocalGameplayCue(const FHitResult& HitResult) const
{
    FGameplayCueParameters CueParam;
    CueParam.Location = HitResult.ImpactPoint;
    CueParam.Normal = HitResult.ImpactNormal;

    for (const FGameplayTag& GameplayCueTag : TriggerGameplayCueTags)
    {
        UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(HitResult.GetActor(), GameplayCueTag, EGameplayCueEvent::Executed, CueParam);
    }
}