// Christopher Naglik All Rights Reserved


#include "Weapon/DisplaySwordEquipComponent.h"

UDisplaySwordEquipComponent::UDisplaySwordEquipComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UDisplaySwordEquipComponent::Initialize(USkeletalMeshComponent* InOwnerMesh, UStaticMeshComponent* InLeftSword, UStaticMeshComponent* InRightSword)
{
    OwnerMesh = InOwnerMesh;
    LeftSword = InLeftSword;
    RightSword = InRightSword;

    // Start with swords in sheath sockets
    if (LeftSword && RightSword && OwnerMesh)
    {
        AttachSwordsToSockets(LeftSheathSocket, RightSheathSocket);
    }

    EquipState = ESwordEquipState::Unequipped;
}

void UDisplaySwordEquipComponent::BeginEquip()
{
    if (EquipState != ESwordEquipState::Unequipped) return;
    EquipState = ESwordEquipState::Equipping;
}

void UDisplaySwordEquipComponent::ExecutePhaseAction(ESwordPhaseAction Action)
{
    switch (Action)
    {
    case ESwordPhaseAction::DetachAndThrow:
        // Infer transition direction from resting state, matching SwordEquipComponent.
      // Needed because the anim notify fires without knowing which ability triggered it.
        if (!IsTransitioning())
        {
            if (EquipState == ESwordEquipState::Unequipped)
                EquipState = ESwordEquipState::Equipping;
            else if (EquipState == ESwordEquipState::Equipped)
                EquipState = ESwordEquipState::Unequipping;
            else
                return;
        }
        DetachSwordsToActorRoot();
        StartPhase(ESwordFlyPhase::Throw);
        break;

    case ESwordPhaseAction::BeginSnap:
        // Allow snap during either transition direction
        if (!IsTransitioning()) return;
        StartPhase(ESwordFlyPhase::Snap);
        break;

    case ESwordPhaseAction::AttachToTarget:
        // Handle both equip and unequip finalization
        if (EquipState == ESwordEquipState::Equipping)
            FinalizeEquip();
        else if (EquipState == ESwordEquipState::Unequipping)
            FinalizeUnequip();
        break;
    }
}

void UDisplaySwordEquipComponent::FinalizeEquip()
{
    CurrentPhase = ESwordFlyPhase::None;
    SetComponentTickEnabled(false);
    AttachSwordsToSockets(LeftHandSocket, RightHandSocket);
    EquipState = ESwordEquipState::Equipped;
}

void UDisplaySwordEquipComponent::AttachSwordsToSockets(FName LeftSocket, FName RightSocket)
{
    if (!OwnerMesh) return;
    if (LeftSword)
        LeftSword->AttachToComponent(OwnerMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, LeftSocket);
    if (RightSword)
        RightSword->AttachToComponent(OwnerMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, RightSocket);
}

void UDisplaySwordEquipComponent::DetachSwordsToActorRoot()
{
    // Parent swords to the mesh instead of root — CharacterDisplay's root is
     // at the camera position (far from the character), so using it would put
     // all transform math in a distant coordinate space and cause drift
    USceneComponent* StableParent = OwnerMesh ? (USceneComponent*)OwnerMesh : GetOwner()->GetRootComponent();

    if (LeftSword)
        LeftSword->AttachToComponent(StableParent, FAttachmentTransformRules::KeepWorldTransform);
    if (RightSword)
        RightSword->AttachToComponent(StableParent, FAttachmentTransformRules::KeepWorldTransform);
}

void UDisplaySwordEquipComponent::StartPhase(ESwordFlyPhase Phase)
{
    CurrentPhase = Phase;
    PhaseAlpha = 0.f;

    if (LeftSword) LeftPhaseStartTransform = LeftSword->GetRelativeTransform();
    if (RightSword) RightPhaseStartTransform = RightSword->GetRelativeTransform();

    bool bIsEquipping = (EquipState == ESwordEquipState::Equipping);

    switch (Phase)
    {
    case ESwordFlyPhase::Throw:
        CurrentPhaseDuration = EquipThrowDuration;
        LeftPhaseEndTransform = GetApexRelativeTransform(true);
        RightPhaseEndTransform = GetApexRelativeTransform(false);
        break;

    case ESwordFlyPhase::Hover:
        CurrentPhaseDuration = EquipHoverDuration;
        LeftPhaseEndTransform = LeftPhaseStartTransform;
        RightPhaseEndTransform = RightPhaseStartTransform;
        break;

    case ESwordFlyPhase::Snap:
        CurrentPhaseDuration = EquipSnapDuration;
        // Snap to hand sockets when equipping, sheath sockets when unequipping
        if (bIsEquipping)
        {
            LeftPhaseEndTransform = GetSocketRelativeToRoot(LeftHandSocket);
            RightPhaseEndTransform = GetSocketRelativeToRoot(RightHandSocket);
        }
        else
        {
            LeftPhaseEndTransform = GetSocketRelativeToRoot(LeftSheathSocket);
            RightPhaseEndTransform = GetSocketRelativeToRoot(RightSheathSocket);
        }
        break;

    default: break;
    }

    SetComponentTickEnabled(true);
}

void UDisplaySwordEquipComponent::AdvanceToNextPhase()
{
    switch (CurrentPhase)
    {
    case ESwordFlyPhase::Throw:
        StartPhase(ESwordFlyPhase::Hover);
        break;
    case ESwordFlyPhase::Hover:
        StartPhase(ESwordFlyPhase::Snap);
        break;
    case ESwordFlyPhase::Snap:
        // Finalize in the correct direction
        if (EquipState == ESwordEquipState::Equipping)
            FinalizeEquip();
        else if (EquipState == ESwordEquipState::Unequipping)
            FinalizeUnequip();
        break;
    default:
        SetComponentTickEnabled(false);
        break;
    }
}
void UDisplaySwordEquipComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (CurrentPhase == ESwordFlyPhase::None)
    {
        SetComponentTickEnabled(false);
        return;
    }

    if (CurrentPhaseDuration > 0.f)
        PhaseAlpha += DeltaTime / CurrentPhaseDuration;
    else
        PhaseAlpha = 1.f;

    if (PhaseAlpha >= 1.f)
    {
        PhaseAlpha = 1.f;
        UpdateSwordTransforms(1.f);
        AdvanceToNextPhase();
        return;
    }

    float EasedAlpha = PhaseAlpha;
    switch (CurrentPhase)
    {
    case ESwordFlyPhase::Throw:
        EasedAlpha = FMath::InterpEaseOut(0.f, 1.f, PhaseAlpha, 2.f);
        break;
    case ESwordFlyPhase::Hover:
        EasedAlpha = PhaseAlpha;
        break;
    case ESwordFlyPhase::Snap:
        EasedAlpha = FMath::InterpEaseIn(0.f, 1.f, PhaseAlpha, 2.f);
        break;
    default: break;
    }

    UpdateSwordTransforms(EasedAlpha);
}

void UDisplaySwordEquipComponent::UpdateSwordTransforms(float EasedAlpha)
{
    if (LeftSword)
    {
        FVector Pos = FMath::Lerp(LeftPhaseStartTransform.GetLocation(), LeftPhaseEndTransform.GetLocation(), EasedAlpha);
        FQuat Rot = FQuat::Slerp(LeftPhaseStartTransform.GetRotation(), LeftPhaseEndTransform.GetRotation(), EasedAlpha);
        LeftSword->SetRelativeLocationAndRotation(Pos, Rot);
    }
    if (RightSword)
    {
        FVector Pos = FMath::Lerp(RightPhaseStartTransform.GetLocation(), RightPhaseEndTransform.GetLocation(), EasedAlpha);
        FQuat Rot = FQuat::Slerp(RightPhaseStartTransform.GetRotation(), RightPhaseEndTransform.GetRotation(), EasedAlpha);
        RightSword->SetRelativeLocationAndRotation(Pos, Rot);
    }
}


// Transform calculations are relative to the mesh component, not root.
// CharacterDisplay's root sits at the camera (far from the character),
// so mesh-relative math keeps everything in the right coordinate space.
FTransform UDisplaySwordEquipComponent::GetSocketRelativeToRoot(FName SocketName) const
{
    if (!OwnerMesh) return FTransform::Identity;

    FTransform SocketWorldTransform = OwnerMesh->GetSocketTransform(SocketName, RTS_World);
    FTransform MeshWorldTransform = OwnerMesh->GetComponentTransform();

    return SocketWorldTransform.GetRelativeTransform(MeshWorldTransform);
}

// Returns the over-the-head apex position where swords hover mid-flight.
// Uses different offsets for equip vs unequip so the Y values can cross.
FTransform UDisplaySwordEquipComponent::GetApexRelativeTransform(bool bIsLeft) const
{
    bool bIsEquipping = (EquipState == ESwordEquipState::Equipping);

    FVector Offset;
    FRotator Rotation;

    if (bIsEquipping)
    {
        Offset = bIsLeft ? EquipApexLeftSword : EquipApexRightSword;
        Rotation = bIsLeft ? EquipApexRotationLeft : EquipApexRotationRight;
    }
    else
    {
        Offset = bIsLeft ? UnequipApexLeftSword : UnequipApexRightSword;
        Rotation = bIsLeft ? UnequipApexRotationLeft : UnequipApexRotationRight;
    }

    return FTransform(Rotation.Quaternion(), Offset, FVector::OneVector);
}

bool UDisplaySwordEquipComponent::IsTransitioning() const
{
    return (EquipState == ESwordEquipState::Equipping || EquipState == ESwordEquipState::Unequipping);
}

void UDisplaySwordEquipComponent::FinalizeUnequip()
{
    CurrentPhase = ESwordFlyPhase::None;
    SetComponentTickEnabled(false);
    AttachSwordsToSockets(LeftSheathSocket, RightSheathSocket);
    EquipState = ESwordEquipState::Unequipped;
}