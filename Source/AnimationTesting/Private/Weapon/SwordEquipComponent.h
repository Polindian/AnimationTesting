// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SwordEquipComponent.generated.h"

// The four possible states the swords can be in
UENUM(BlueprintType)
enum class ESwordEquipState : uint8
{
    Unequipped,   // Swords are in sheath sockets (LeftCover / RightCover)
    Equipping,    // Mid-transition: sheath -> hands
    Equipped,     // Swords are in hand sockets (sword_left / sword_right)
    Unequipping   // Mid-transition: hands -> sheath
};

// The phases of the flying interpolation
// Each transition (equip or unequip) goes through 3 phases
UENUM(BlueprintType)
enum class ESwordFlyPhase : uint8
{
    None,
    Throw,   // Detach from source socket, lerp to apex (over-shoulder position)
    Hover,   // Brief pause at the apex
    Snap     // Quick lerp from apex into target socket, then reattach
};

// Enum used by the anim notify to tell us what to do at each keyframe
UENUM(BlueprintType)
enum class ESwordPhaseAction : uint8
{
    DetachAndThrow,  // Detach swords from current socket, begin Throw phase
    BeginSnap,       // Skip straight to the Snap phase (called after hover is done naturally, or forced by notify)
    AttachToTarget   // Immediately finish: attach swords to target sockets
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class USwordEquipComponent : public UActorComponent
{
	GENERATED_BODY()

    public:
        USwordEquipComponent();
        virtual void BeginPlay() override;
        virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

        // Called by the GA_EquipToggle ability to know which montage to play
        UFUNCTION(BlueprintCallable)
        ESwordEquipState GetEquipState() const { return EquipState; }

        // Called by the ability to kick off the transition
        // (The actual flying starts later, when the anim notify fires)
        void BeginEquip();
        void BeginUnequip();

        // Called by AN_SwordPhase anim notify at specific montage keyframes
        void ExecutePhaseAction(ESwordPhaseAction Action);

        // Called when the whole transition finishes (montage end or snap complete)
        void FinalizeEquip();
        void FinalizeUnequip();

private:
    // ---------- Socket names ----------
    UPROPERTY(EditDefaultsOnly, Category = "Swords|Sockets")
    FName LeftHandSocket = "sword_left";

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Sockets")
    FName RightHandSocket = "sword_right";

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Sockets")
    FName LeftSheathSocket = "LeftCover";

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Sockets")
    FName RightSheathSocket = "RightCover";

    // ---------- Component tags to find swords at BeginPlay ----------
    UPROPERTY(EditDefaultsOnly, Category = "Swords|Setup")
    FName LeftSwordComponentTag = "LeftSword";

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Setup")
    FName RightSwordComponentTag = "RightSword";

    // ---------- EQUIP flying settings (sheath -> shoulders -> hands) ----------
    // Each sword gets its own apex offset and rotation so they can cross
    UPROPERTY(EditDefaultsOnly, Category = "Swords|Equip Apex")
    FVector EquipApexLeftSword = FVector(-10.f, 15.f, 80.f);

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Equip Apex")
    FRotator EquipApexRotationLeft = FRotator(0.f, 30.f, 90.f);

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Equip Apex")
    FVector EquipApexRightSword = FVector(-10.f, -15.f, 80.f);

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Equip Apex")
    FRotator EquipApexRotationRight = FRotator(0.f, -30.f, -90.f);

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Equip Timing")
    float EquipThrowDuration = 0.7f;

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Equip Timing")
    float EquipHoverDuration = 0.4f;

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Equip Timing")
    float EquipSnapDuration = 0.3f;

    // ---------- UNEQUIP flying settings (hands -> shoulders -> sheath) ----------
    UPROPERTY(EditDefaultsOnly, Category = "Swords|Unequip Apex")
    FVector UnequipApexLeftSword = FVector(-15.f, -20.f, 75.f);

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Unequip Apex")
    FRotator UnequipApexRotationLeft = FRotator(0.f, -30.f, -90.f);

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Unequip Apex")
    FVector UnequipApexRightSword = FVector(-15.f, 20.f, 75.f);

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Unequip Apex")
    FRotator UnequipApexRotationRight = FRotator(0.f, 30.f, 90.f);

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Unequip Timing")
    float UnequipThrowDuration = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Unequip Timing")
    float UnequipHoverDuration = 0.3f;

    UPROPERTY(EditDefaultsOnly, Category = "Swords|Unequip Timing")
    float UnequipSnapDuration = 0.3f;

    // ---------- State ----------
    ESwordEquipState EquipState = ESwordEquipState::Unequipped;
    ESwordFlyPhase CurrentPhase = ESwordFlyPhase::None;
    float PhaseAlpha = 0.f;
    float CurrentPhaseDuration = 0.f;

    FTransform LeftPhaseStartTransform;
    FTransform LeftPhaseEndTransform;
    FTransform RightPhaseStartTransform;
    FTransform RightPhaseEndTransform;

    // ---------- References ----------
    UPROPERTY()
    UStaticMeshComponent* LeftSword = nullptr;

    UPROPERTY()
    UStaticMeshComponent* RightSword = nullptr;

    UPROPERTY()
    USkeletalMeshComponent* OwnerMesh = nullptr;

    // ---------- Internal helpers ----------
    void FindSwords();
    void AttachSwordsToSockets(FName LeftSocket, FName RightSocket);
    void DetachSwordsToMeshRoot();
    void StartPhase(ESwordFlyPhase Phase);
    void AdvanceToNextPhase();
    FTransform GetSocketRelativeToRoot(FName SocketName) const;
    FTransform GetApexRelativeTransform(bool bIsLeft) const;
    void UpdateSwordTransforms(float EasedAlpha);
    void UpdateEquippedTag();
    bool IsTransitioning() const;
};
