// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "GenericTeamAgentInterface.h"
#include "Framework/Flag.h"
#include "BTService_UpdateGoal.generated.h"

class AFlag;

UCLASS()
class UBTService_UpdateGoal : public UBTService
{
    GENERATED_BODY()

public:
    UBTService_UpdateGoal();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
    // How much enemy capture % on a zone triggers a reaction
    UPROPERTY(EditAnywhere, Category = "Goal")
    float TerritoryThreshold = 50.f;

    // ======================================================
    // SMART DISTRIBUTION: Maximum ratio of alive allies that
    // should be assigned to a single flag. If a flag already
    // has this % of allies heading to it, excess skeletons
    // get redirected to the next priority flag.
    //
    // Example: 0.8 means max 80% of your alive skeletons
    // can go to one flag. The other 20% go elsewhere.
    // ======================================================
    UPROPERTY(EditAnywhere, Category = "Goal|Distribution", meta = (ClampMin = "0.3", ClampMax = "1.0"))
    float MaxPerFlagRatio = 0.8f;

    // Minimum number of skeletons before distribution kicks in with only 2-3 skeletons alive
    UPROPERTY(EditAnywhere, Category = "Goal|Distribution")
    int32 MinSkeletonsForDistribution = 4;

    // Blackboard key names
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FName GoalKeyName = "Goal";

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FName LockedToMidZoneKeyName = "bLockedToMidZone";

    // Cached flag references (found once on first tick)
    UPROPERTY()
    AFlag* Zone1Flag = nullptr;

    UPROPERTY()
    AFlag* Zone2Flag = nullptr;

    UPROPERTY()
    AFlag* Zone3Flag = nullptr;

    bool bFlagsCached = false;

    void CacheFlags(UWorld* World);

    // Returns the base flag for a given team
    AFlag* GetMyBaseFlag(uint8 TeamId) const;

    // Returns the enemy's base flag
    AFlag* GetEnemyBaseFlag(uint8 TeamId) const;

    // Returns the EFlagOwnership enum value that represents this team
    EFlagOwnership GetTeamOwnership(uint8 TeamId) const;

    // Returns how much "enemy progress" is on a flag from this team's perspective
    float GetEnemyProgress(AFlag* Flag, uint8 MyTeamId) const;

    // Returns how much "friendly progress" is on a flag
    float GetFriendlyProgress(AFlag* Flag, uint8 MyTeamId) const;

    // ======================================================
    // DISTRIBUTION HELPERS
    // ======================================================

    // Counts how many allied AI controllers currently have their goal blackboard key set to the given flag
    int32 CountAlliesAssignedTo(AFlag* Flag, uint8 MyTeamId, AAIController* ExcludeSelf) const;

    // Returns the total number of alive allied AI skeletons.
    int32 GetTotalAliveAllies(uint8 MyTeamId) const;

    // Given that the primary goal is overcrowded, pick the next best flag to send this skeleton to.
    AFlag* GetSecondaryGoal(AFlag* PrimaryGoal, uint8 MyTeamId) const;
};