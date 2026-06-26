// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapon/SwordEquipComponent.h" // Reuse the enums
#include "DisplaySwordEquipComponent.generated.h"

// Stripped-down version of SwordEquipComponent for CharacterDisplay.
// Same Throw→Hover→Snap flying interpolation, but no ACharacter/GAS dependency.
// Triggered by anim notifies on the preview animation.
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UDisplaySwordEquipComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDisplaySwordEquipComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Call this after ConfigureWithCharacterDefinition to give us the weapon refs
    void Initialize(USkeletalMeshComponent* InOwnerMesh, UStaticMeshComponent* InLeftSword, UStaticMeshComponent* InRightSword);

    // Called by anim notify to drive the flying phases (same as SwordEquipComponent)
    void ExecutePhaseAction(ESwordPhaseAction Action);

    void BeginEquip();
    void FinalizeEquip();

    void FinalizeUnequip();

private:
    // Socket names (same defaults as SwordEquipComponent)
    UPROPERTY(EditDefaultsOnly, Category = "Swords|Sockets")
    FName LeftHandSocket = "sword_left";

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Sockets")
    FName RightHandSocket = "sword_right";

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Sockets")
    FName LeftSheathSocket = "LeftCover";

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Sockets")
    FName RightSheathSocket = "RightCover";

    // ---------- EQUIP apex ----------
    UPROPERTY(EditDefaultsOnly, Category = "Swords|Equip Apex")
    FVector EquipApexLeftSword = FVector(0.f, -15.f, 200.f);

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Equip Apex")
    FRotator EquipApexRotationLeft = FRotator(10.f, 0.f, 90.f);

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Equip Apex")
    FVector EquipApexRightSword = FVector(0.f, -15.f, 200.f);

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Equip Apex")
    FRotator EquipApexRotationRight = FRotator(-10.f, 0.f, 90.f);

    // ---------- UNEQUIP apex ----------
    UPROPERTY(EditDefaultsOnly, Category = "Swords|Unequip Apex")
    FVector UnequipApexLeftSword = FVector(0.f, -15.f, 200.f);

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Unequip Apex")
    FRotator UnequipApexRotationLeft = FRotator(10.f, 0.f, 90.f);

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Unequip Apex")
    FVector UnequipApexRightSword = FVector(0.f, -15.f, 200.f);

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Unequip Apex")
    FRotator UnequipApexRotationRight = FRotator(10.f, 0.f, 90.f);

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Equip Timing")
    float EquipThrowDuration = 0.7f;

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Equip Timing")
    float EquipHoverDuration = 0.4f;

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Equip Timing")
    float EquipSnapDuration = 0.3f;

    bool IsTransitioning() const;

    // State
    ESwordEquipState EquipState = ESwordEquipState::Unequipped;
    ESwordFlyPhase CurrentPhase = ESwordFlyPhase::None;
    float PhaseAlpha = 0.f;
    float CurrentPhaseDuration = 0.f;

    FTransform LeftPhaseStartTransform;
    FTransform LeftPhaseEndTransform;
    FTransform RightPhaseStartTransform;
    FTransform RightPhaseEndTransform;

    UPROPERTY()
    UStaticMeshComponent* LeftSword = nullptr;

    UPROPERTY()
    UStaticMeshComponent* RightSword = nullptr;

    UPROPERTY()
    USkeletalMeshComponent* OwnerMesh = nullptr;

    void AttachSwordsToSockets(FName LeftSocket, FName RightSocket);
    void DetachSwordsToActorRoot();
    void StartPhase(ESwordFlyPhase Phase);
    void AdvanceToNextPhase();
    FTransform GetSocketRelativeToRoot(FName SocketName) const;
    FTransform GetApexRelativeTransform(bool bIsLeft) const;
    void UpdateSwordTransforms(float EasedAlpha);
};