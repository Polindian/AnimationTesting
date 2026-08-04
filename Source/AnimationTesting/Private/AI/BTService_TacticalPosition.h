// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_TacticalPosition.generated.h"

class AFlag;
class AAIController;

UCLASS()
class UBTService_TacticalPosition : public UBTService
{
    GENERATED_BODY()

public:
    UBTService_TacticalPosition();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
    // ======================================================
    // BLACKBOARD KEYS
    // ======================================================

    // The vector position the skeleton should move to when
    // approaching a target (offset to create flanking angles)
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FName FlankPositionKeyName = "FlankPosition";

    // The vector position the skeleton should move to when
    // heading to a goal flag (spread out or varied path)
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FName SpreadPositionKeyName = "SpreadPosition";

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FName TargetKeyName = "Target";

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FName GoalKeyName = "Goal";

    // ======================================================
    // FLANK POSITIONING SETTINGS
    // Each skeleton is assigned a unique angle slice around
    // the target based on how many allies share that target.
    // ======================================================

    // How far from the target the flank position is placed
    UPROPERTY(EditAnywhere, Category = "Tactical|Flanking")
    float FlankDistance = 150.f;

    // Minimum angle offset from direct approach (degrees).
    // First skeleton gets 0 (head-on), others get spread out.
    UPROPERTY(EditAnywhere, Category = "Tactical|Flanking")
    float MinFlankAngle = 60.f;

    // ======================================================
    // SPREAD FORMATION SETTINGS
    // When skeletons are inside a flag zone with no enemies,
    // they spread out evenly
    // ======================================================

    // How far from the flag center to spread 
    UPROPERTY(EditAnywhere, Category = "Tactical|Spread")
    float SpreadRadiusRatio = 0.6f;

    // ======================================================
    // PATH VARIETY SETTINGS
    // When moving to a flag zone, each
    // skeleton gets a random offset so they don't all take
    // the exact same path.
    // ======================================================

    // Maximum lateral offset from the flag center when
    // choosing a destination point within the zone.
    UPROPERTY(EditAnywhere, Category = "Tactical|PathVariety")
    float PathOffsetMax = 250.f;

    // ======================================================
    // FLAG CACHING
    // ======================================================

    UPROPERTY()
    TArray<AFlag*> AllFlags;
    bool bFlagsCached = false;

    void CacheFlags(UWorld* World);

    // ======================================================
    // HELPERS
    // ======================================================

    // Gets all allied AI controllers that share the same target.
    // Returns the total count and this skeleton's index in that group.
    void GetFlankSlot(AAIController* Self, AActor* Target, uint8 MyTeamId,
        int32& OutSlotIndex, int32& OutTotalCount) const;

    // Gets all allied AI controllers inside the same flag zone.
    // Returns this skeleton's index and total count for spread.
    void GetSpreadSlot(AAIController* Self, AFlag* Flag, uint8 MyTeamId,
        int32& OutSlotIndex, int32& OutTotalCount) const;

    // Returns a consistent random offset for a given pawn
    FVector GetConsistentOffset(APawn* Pawn, float MaxRadius) const;
};