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
        if (EquipState == ESwordEquipState::Unequipped)
            EquipState = ESwordEquipState::Equipping;
        DetachSwordsToActorRoot();
        StartPhase(ESwordFlyPhase::Throw);
        break;

    case ESwordPhaseAction::BeginSnap:
        if (EquipState != ESwordEquipState::Equipping) return;
        StartPhase(ESwordFlyPhase::Snap);
        break;

    case ESwordPhaseAction::AttachToTarget:
        if (EquipState == ESwordEquipState::Equipping)
            FinalizeEquip();
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
    USceneComponent* Root = GetOwner()->GetRootComponent();
    if (LeftSword)
        LeftSword->AttachToComponent(Root, FAttachmentTransformRules::KeepWorldTransform);
    if (RightSword)
        RightSword->AttachToComponent(Root, FAttachmentTransformRules::KeepWorldTransform);
}

void UDisplaySwordEquipComponent::StartPhase(ESwordFlyPhase Phase)
{
    CurrentPhase = Phase;
    PhaseAlpha = 0.f;

    if (LeftSword) LeftPhaseStartTransform = LeftSword->GetRelativeTransform();
    if (RightSword) RightPhaseStartTransform = RightSword->GetRelativeTransform();

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
        LeftPhaseEndTransform = GetSocketRelativeToRoot(LeftHandSocket);
        RightPhaseEndTransform = GetSocketRelativeToRoot(RightHandSocket);
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
        FinalizeEquip();
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

FTransform UDisplaySwordEquipComponent::GetSocketRelativeToRoot(FName SocketName) const
{
    if (!OwnerMesh || !GetOwner()) return FTransform::Identity;

    FTransform SocketWorldTransform = OwnerMesh->GetSocketTransform(SocketName, RTS_World);
    FTransform RootWorldTransform = GetOwner()->GetRootComponent()->GetComponentTransform();

    return SocketWorldTransform.GetRelativeTransform(RootWorldTransform);
}

FTransform UDisplaySwordEquipComponent::GetApexRelativeTransform(bool bIsLeft) const
{
    FVector Offset = bIsLeft ? EquipApexLeftSword : EquipApexRightSword;
    FRotator Rotation = bIsLeft ? EquipApexRotationLeft : EquipApexRotationRight;
    return FTransform(Rotation.Quaternion(), Offset, FVector::OneVector);
}