// Christopher Naglik All Rights Reserved


#include "Weapon/AN_SwordPhase.h"
#include "Weapon/DisplaySwordEquipComponent.h"

void UAN_SwordPhase::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp || !MeshComp->GetOwner()) return;

    // Try gameplay component first (ACharacter in-match)
    USwordEquipComponent* SwordComp = MeshComp->GetOwner()->FindComponentByClass<USwordEquipComponent>();
    if (SwordComp)
    {
        SwordComp->ExecutePhaseAction(Action);
        return;
    }

    // Fall back to display component (CharacterDisplay in lobby)
    UDisplaySwordEquipComponent* DisplayComp = MeshComp->GetOwner()->FindComponentByClass<UDisplaySwordEquipComponent>();
    if (DisplayComp)
    {
        DisplayComp->ExecutePhaseAction(Action);
    }
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