// Christopher Naglik All Rights Reserved

#include "Animations/AnimNotifyState_ThrustTrail.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotifyState_ThrustTrail::NotifyBegin(USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation, float TotalDuration,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    if (!MeshComp || !TrailSystem) return;

    // Cosmetic only — nothing to draw on a dedicated server
    if (MeshComp->GetWorld() && MeshComp->GetWorld()->IsNetMode(NM_DedicatedServer)) return;

    UNiagaraComponent* Trail = UNiagaraFunctionLibrary::SpawnSystemAttached(
        TrailSystem, MeshComp, NAME_None,
        FVector::ZeroVector, FRotator::ZeroRotator,
        EAttachLocation::SnapToTarget, false);

    if (Trail)
    {
        // This is the whole integration: tell the data interface which mesh to snapshot
        UNiagaraFunctionLibrary::OverrideSystemUserVariableSkeletalMeshComponent(
            Trail, SkeletalMeshParameterName, MeshComp);
    }
}

void UAnimNotifyState_ThrustTrail::NotifyEnd(USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    if (!MeshComp) return;

    // Find the trail we spawned among the mesh's attached children
    TArray<USceneComponent*> Children;
    MeshComp->GetChildrenComponents(false, Children);

    for (USceneComponent* Child : Children)
    {
        UNiagaraComponent* Niagara = Cast<UNiagaraComponent>(Child);
        if (Niagara && Niagara->GetAsset() == TrailSystem)
        {
            Niagara->SetAutoDestroy(true);
            Niagara->Deactivate();   // lets existing snapshots dissolve out
        }
    }
}