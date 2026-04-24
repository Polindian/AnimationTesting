// Christopher Naglik All Rights Reserved


#include "Weapon/AN_SwordPhase.h"

void UAN_SwordPhase::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp || !MeshComp->GetOwner()) return;

    // Find the SwordEquipComponent on the owning actor
    USwordEquipComponent* SwordComp = MeshComp->GetOwner()->FindComponentByClass<USwordEquipComponent>();
    if (!SwordComp) return;

    SwordComp->ExecutePhaseAction(Action);
}

FString UAN_SwordPhase::GetNotifyName_Implementation() const
{
    switch (Action)
    {
    case ESwordPhaseAction::DetachAndThrow: return TEXT("Sword: Detach & Throw");
    case ESwordPhaseAction::BeginSnap:      return TEXT("Sword: Begin Snap");
    case ESwordPhaseAction::AttachToTarget: return TEXT("Sword: Attach to Target");
    default: return TEXT("Sword: Unknown");
    }
}