// Christopher Naglik All Rights Reserved


#include "Weapon/SwordEquipComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAS/ChrisAbilitySystemStatics.h"

USwordEquipComponent::USwordEquipComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void USwordEquipComponent::BeginPlay()
{
    Super::BeginPlay();

    FindSwords();

    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (OwnerChar)
    {
        OwnerMesh = OwnerChar->GetMesh();
    }

    if (LeftSword && RightSword && OwnerMesh)
    {
        AttachSwordsToSockets(LeftSheathSocket, RightSheathSocket);
    }

    UpdateEquippedTag();
}

void USwordEquipComponent::FindSwords()
{
    TArray<UActorComponent*> AllComponents = GetOwner()->GetComponents().Array();
    for (UActorComponent* Comp : AllComponents)
    {
        if (UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(Comp))
        {
            if (MeshComp->ComponentHasTag(LeftSwordComponentTag))
            {
                LeftSword = MeshComp;
            }
            else if (MeshComp->ComponentHasTag(RightSwordComponentTag))
            {
                RightSword = MeshComp;
            }
        }
    }

    if (!LeftSword) UE_LOG(LogTemp, Warning, TEXT("SwordEquipComponent: Could not find LeftSword by tag '%s'"), *LeftSwordComponentTag.ToString());
    if (!RightSword) UE_LOG(LogTemp, Warning, TEXT("SwordEquipComponent: Could not find RightSword by tag '%s'"), *RightSwordComponentTag.ToString());
}

// -----------------------------------------------------------------------
// State helpers
// -----------------------------------------------------------------------

bool USwordEquipComponent::IsTransitioning() const
{
    return (EquipState == ESwordEquipState::Equipping || EquipState == ESwordEquipState::Unequipping);
}

// -----------------------------------------------------------------------
// Public interface
// -----------------------------------------------------------------------

void USwordEquipComponent::BeginEquip()
{
    if (EquipState != ESwordEquipState::Unequipped) return;
    EquipState = ESwordEquipState::Equipping;
    UpdateEquippedTag();
}

void USwordEquipComponent::BeginUnequip()
{
    if (EquipState != ESwordEquipState::Equipped) return;
    EquipState = ESwordEquipState::Unequipping;
    UpdateEquippedTag();
}

void USwordEquipComponent::ExecutePhaseAction(ESwordPhaseAction Action)
{
    switch (Action)
    {
    case ESwordPhaseAction::DetachAndThrow:
        // If we're not already transitioning (e.g. on a remote client where
        // BeginEquip/BeginUnequip was never called from the ability),
        // infer the transition direction from the current resting state.
        // This works because the montage is replicated by GAS, so anim
        // notifies fire on ALL clients — we just need the state to be correct.
        if (!IsTransitioning())
        {
            if (EquipState == ESwordEquipState::Unequipped)
                EquipState = ESwordEquipState::Equipping;
            else if (EquipState == ESwordEquipState::Equipped)
                EquipState = ESwordEquipState::Unequipping;
            else
                return;
        }
        DetachSwordsToMeshRoot();
        StartPhase(ESwordFlyPhase::Throw);
        break;

    case ESwordPhaseAction::BeginSnap:
        if (!IsTransitioning()) return;
        StartPhase(ESwordFlyPhase::Snap);
        break;

    case ESwordPhaseAction::AttachToTarget:
        if (!IsTransitioning()) return;
        if (EquipState == ESwordEquipState::Equipping)
            FinalizeEquip();
        else if (EquipState == ESwordEquipState::Unequipping)
            FinalizeUnequip();
        break;
    }
}

void USwordEquipComponent::FinalizeEquip()
{
    CurrentPhase = ESwordFlyPhase::None;
    SetComponentTickEnabled(false);

    AttachSwordsToSockets(LeftHandSocket, RightHandSocket);
    EquipState = ESwordEquipState::Equipped;
    UpdateEquippedTag();

    UE_LOG(LogTemp, Log, TEXT("SwordEquip: FinalizeEquip — swords attached to hand sockets"));
}

void USwordEquipComponent::FinalizeUnequip()
{
    CurrentPhase = ESwordFlyPhase::None;
    SetComponentTickEnabled(false);

    AttachSwordsToSockets(LeftSheathSocket, RightSheathSocket);
    EquipState = ESwordEquipState::Unequipped;
    UpdateEquippedTag();

    UE_LOG(LogTemp, Log, TEXT("SwordEquip: FinalizeUnequip — swords attached to sheath sockets"));
}

// -----------------------------------------------------------------------
// Attachment helpers
// -----------------------------------------------------------------------

void USwordEquipComponent::AttachSwordsToSockets(FName LeftSocket, FName RightSocket)
{
    if (!OwnerMesh) return;

    if (LeftSword)
    {
        LeftSword->AttachToComponent(OwnerMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, LeftSocket);
    }
    if (RightSword)
    {
        RightSword->AttachToComponent(OwnerMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, RightSocket);
    }
}

void USwordEquipComponent::DetachSwordsToMeshRoot()
{
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerChar) return;

    USceneComponent* StableRoot = OwnerChar->GetRootComponent();

    if (LeftSword)
    {
        LeftSword->AttachToComponent(StableRoot, FAttachmentTransformRules::KeepWorldTransform);
    }
    if (RightSword)
    {
        RightSword->AttachToComponent(StableRoot, FAttachmentTransformRules::KeepWorldTransform);
    }
}

// -----------------------------------------------------------------------
// Phase management
// -----------------------------------------------------------------------

void USwordEquipComponent::StartPhase(ESwordFlyPhase Phase)
{
    CurrentPhase = Phase;
    PhaseAlpha = 0.f;

    if (LeftSword)
        LeftPhaseStartTransform = LeftSword->GetRelativeTransform();
    if (RightSword)
        RightPhaseStartTransform = RightSword->GetRelativeTransform();

    bool bIsEquipping = (EquipState == ESwordEquipState::Equipping);

    switch (Phase)
    {
    case ESwordFlyPhase::Throw:
        CurrentPhaseDuration = bIsEquipping ? EquipThrowDuration : UnequipThrowDuration;
        LeftPhaseEndTransform = GetApexRelativeTransform(true);
        RightPhaseEndTransform = GetApexRelativeTransform(false);
        break;

    case ESwordFlyPhase::Hover:
        CurrentPhaseDuration = bIsEquipping ? EquipHoverDuration : UnequipHoverDuration;
        LeftPhaseEndTransform = LeftPhaseStartTransform;
        RightPhaseEndTransform = RightPhaseStartTransform;
        break;

    case ESwordFlyPhase::Snap:
    {
        CurrentPhaseDuration = bIsEquipping ? EquipSnapDuration : UnequipSnapDuration;
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
    }
    default:
        break;
    }

    SetComponentTickEnabled(true);
}

void USwordEquipComponent::AdvanceToNextPhase()
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

// -----------------------------------------------------------------------
// Tick
// -----------------------------------------------------------------------

void USwordEquipComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (CurrentPhase == ESwordFlyPhase::None)
    {
        SetComponentTickEnabled(false);
        return;
    }

    if (CurrentPhaseDuration > 0.f)
    {
        PhaseAlpha += DeltaTime / CurrentPhaseDuration;
    }
    else
    {
        PhaseAlpha = 1.f;
    }

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
    default:
        break;
    }

    UpdateSwordTransforms(EasedAlpha);
}

void USwordEquipComponent::UpdateSwordTransforms(float EasedAlpha)
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

// -----------------------------------------------------------------------
// Transform helpers
// -----------------------------------------------------------------------

FTransform USwordEquipComponent::GetSocketRelativeToRoot(FName SocketName) const
{
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerMesh || !OwnerChar) return FTransform::Identity;

    FTransform SocketWorldTransform = OwnerMesh->GetSocketTransform(SocketName, RTS_World);
    FTransform RootWorldTransform = OwnerChar->GetRootComponent()->GetComponentTransform();

    return SocketWorldTransform.GetRelativeTransform(RootWorldTransform);
}

FTransform USwordEquipComponent::GetApexRelativeTransform(bool bIsLeft) const
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

// -----------------------------------------------------------------------
// Tag management
// -----------------------------------------------------------------------

void USwordEquipComponent::UpdateEquippedTag()
{
    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
    if (!ASC) return;

    FGameplayTag EquippedTag = UChrisAbilitySystemStatics::GetWeaponsEquippedTag();
    FGameplayTag UnequippedTag = UChrisAbilitySystemStatics::GetWeaponsUnequippedTag();

    if (EquipState == ESwordEquipState::Equipped)
    {
        ASC->AddLooseGameplayTag(EquippedTag);
        ASC->RemoveLooseGameplayTag(UnequippedTag);
    }
    else if (EquipState == ESwordEquipState::Unequipped)
    {
        ASC->RemoveLooseGameplayTag(EquippedTag);
        ASC->AddLooseGameplayTag(UnequippedTag);
    }
    else
    {
        // Equipping or Unequipping — remove BOTH tags (transitioning)
        ASC->RemoveLooseGameplayTag(EquippedTag);
        ASC->RemoveLooseGameplayTag(UnequippedTag);
    }
}
