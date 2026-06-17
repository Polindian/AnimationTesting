// Christopher Naglik All Rights Reserved

#include "AI/BTService_UpdateGoal.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Framework/Flag.h"
#include "EngineUtils.h"

UBTService_UpdateGoal::UBTService_UpdateGoal()
{
    NodeName = "Update Goal";

    // Ticks every 0.5s — flag state doesn't change rapidly
    Interval = 0.5f;
    RandomDeviation = 0.1f;
}

void UBTService_UpdateGoal::CacheFlags(UWorld* World)
{
    if (!World) return;

    for (TActorIterator<AFlag> It(World); It; ++It)
    {
        AFlag* Flag = *It;
        switch (Flag->GetZoneID())
        {
        case 1: Zone1Flag = Flag; break;
        case 2: Zone2Flag = Flag; break;
        case 3: Zone3Flag = Flag; break;
        }
    }

    bFlagsCached = (Zone1Flag && Zone2Flag && Zone3Flag);

    if (!bFlagsCached)
    {
        UE_LOG(LogTemp, Error, TEXT("[BTService_UpdateGoal] Could not find all 3 flags! Z1:%d Z2:%d Z3:%d"),
            Zone1Flag != nullptr, Zone2Flag != nullptr, Zone3Flag != nullptr);
    }
}

AFlag* UBTService_UpdateGoal::GetMyBaseFlag(uint8 TeamId) const
{
    return (TeamId == 0) ? Zone1Flag : Zone3Flag;
}

AFlag* UBTService_UpdateGoal::GetEnemyBaseFlag(uint8 TeamId) const
{
    return (TeamId == 0) ? Zone3Flag : Zone1Flag;
}

EFlagOwnership UBTService_UpdateGoal::GetTeamOwnership(uint8 TeamId) const
{
    return (TeamId == 0) ? EFlagOwnership::TeamOne : EFlagOwnership::TeamTwo;
}

float UBTService_UpdateGoal::GetEnemyProgress(AFlag* Flag, uint8 MyTeamId) const
{
    if (!Flag || Flag->IsCaptured()) return 0.f;
    if (Flag->GetOwnership() == EFlagOwnership::Neutral) return 0.f;

    EFlagOwnership EnemyOwnership = (MyTeamId == 0) ? EFlagOwnership::TeamTwo : EFlagOwnership::TeamOne;
    if (Flag->GetOwnership() == EnemyOwnership)
    {
        return Flag->GetCapturePercent();
    }

    return 0.f;
}

float UBTService_UpdateGoal::GetFriendlyProgress(AFlag* Flag, uint8 MyTeamId) const
{
    if (!Flag || Flag->IsCaptured()) return 0.f;
    if (Flag->GetOwnership() == EFlagOwnership::Neutral) return 0.f;

    EFlagOwnership MyOwnership = GetTeamOwnership(MyTeamId);
    if (Flag->GetOwnership() == MyOwnership)
    {
        return Flag->GetCapturePercent();
    }
    else
    {
        return -Flag->GetCapturePercent();
    }
}

// ======================================================
// COUNT ALLIES ASSIGNED TO A FLAG
//
// Iterates all AI controllers in the world that share our
// team. Reads their blackboard "Goal" key to see which flag
// they're currently heading toward (exclude self)
// ======================================================
int32 UBTService_UpdateGoal::CountAlliesAssignedTo(AFlag* Flag, uint8 MyTeamId, AAIController* ExcludeSelf) const
{
    if (!Flag) return 0;

    int32 Count = 0;
    UWorld* World = ExcludeSelf->GetWorld();
    if (!World) return 0;

    for (TActorIterator<AAIController> It(World); It; ++It)
    {
        AAIController* OtherAIC = *It;
        if (!OtherAIC || OtherAIC == ExcludeSelf) continue;

        // Only count allies on the same team
        if (OtherAIC->GetGenericTeamId().GetId() != MyTeamId) continue;

        // Check if their pawn is alive
        APawn* OtherPawn = OtherAIC->GetPawn();
        if (!OtherPawn) continue;

        // Read their blackboard Goal key
        UBlackboardComponent* OtherBB = OtherAIC->GetBlackboardComponent();
        if (!OtherBB) continue;

        UObject* OtherGoal = OtherBB->GetValueAsObject(GoalKeyName);
        if (OtherGoal == Flag)
        {
            Count++;
        }
    }

    return Count;
}

// ======================================================
// GET TOTAL ALIVE ALLIES
// Counts all AI controllers on our team that are currently valid
// ======================================================
int32 UBTService_UpdateGoal::GetTotalAliveAllies(uint8 MyTeamId) const
{
    int32 Count = 0;

    UWorld* World = Zone1Flag ? Zone1Flag->GetWorld() : nullptr;
    if (!World) return 0;

    for (TActorIterator<AAIController> It(World); It; ++It)
    {
        AAIController* OtherAIC = *It;
        if (!OtherAIC) continue;
        if (OtherAIC->GetGenericTeamId().GetId() != MyTeamId) continue;
        if (!OtherAIC->GetPawn()) continue;

        Count++;
    }

    return Count;
}

// ======================================================
// GET SECONDARY GOAL
//
// When the primary goal is overcrowded, this picks the
// next best flag. Priority order:
//   1. Zone 2 (mid) — always strategically valuable
//   2. My base flag — defend home
//   3. Enemy base flag — push offense
//
// Skips the PrimaryGoal and any fully captured flags.
// ======================================================
AFlag* UBTService_UpdateGoal::GetSecondaryGoal(AFlag* PrimaryGoal, uint8 MyTeamId) const
{
    // Build priority list: mid > my base > enemy base
    AFlag* Priorities[3] = { Zone2Flag, GetMyBaseFlag(MyTeamId), GetEnemyBaseFlag(MyTeamId) };

    for (AFlag* Candidate : Priorities)
    {
        // Skip the overcrowded primary goal
        if (Candidate == PrimaryGoal) continue;

        // Skip fully captured flags
        if (Candidate && !Candidate->IsCaptured())
        {
            return Candidate;
        }
    }

    // Fallback: if everything is captured, just go to mid
    return Zone2Flag;
}

void UBTService_UpdateGoal::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    // ======================================================
    // SETUP: Get controller, blackboard, team, and cache flags
    // ======================================================
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!AIC) return;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return;

    if (!bFlagsCached)
    {
        CacheFlags(AIC->GetWorld());
        if (!bFlagsCached) return;
    }

    if (!IsValid(Zone1Flag) || !IsValid(Zone2Flag) || !IsValid(Zone3Flag))
    {
        bFlagsCached = false;
        CacheFlags(AIC->GetWorld());
        if (!bFlagsCached) return;
    }

    // Get the AI's team ID (0 = Red, 1 = Blue)
    uint8 MyTeamId = AIC->GetGenericTeamId().GetId();

    AFlag* MyBaseFlag = GetMyBaseFlag(MyTeamId);
    AFlag* EnemyBaseFlag = GetEnemyBaseFlag(MyTeamId);
    AFlag* MidFlag = Zone2Flag;

    // ======================================================
    // COUNT CAPTURED FLAGS
    // ======================================================
    int32 CapturedCount = 0;
    AFlag* FlagArray[3] = { Zone1Flag, Zone2Flag, Zone3Flag };
    AFlag* LastUncapturedFlag = nullptr;

    for (AFlag* F : FlagArray)
    {
        if (F->IsCaptured())
        {
            CapturedCount++;
        }
        else
        {
            LastUncapturedFlag = F;
        }
    }

    AFlag* ChosenGoal = nullptr;

    // ======================================================
    // POINT 3: Two flags captured — go to the last remaining one
    // ======================================================
    if (CapturedCount >= 2 && LastUncapturedFlag)
    {
        ChosenGoal = LastUncapturedFlag;
    }
    // ======================================================
    // POINT 2: One flag captured
    // ======================================================
    else if (CapturedCount == 1)
    {
        BB->SetValueAsBool(LockedToMidZoneKeyName, false);

        if (MyBaseFlag->IsCaptured())
        {
            ChosenGoal = !MidFlag->IsCaptured() ? MidFlag : EnemyBaseFlag;
        }
        else if (MidFlag->IsCaptured())
        {
            EFlagOwnership MyOwnership = GetTeamOwnership(MyTeamId);

            if (MidFlag->GetOwnership() == MyOwnership)
            {
                ChosenGoal = !MyBaseFlag->IsCaptured() ? MyBaseFlag : EnemyBaseFlag;
            }
            else
            {
                if (!MyBaseFlag->IsCaptured() && !EnemyBaseFlag->IsCaptured())
                {
                    float MyBaseProgress = GetFriendlyProgress(MyBaseFlag, MyTeamId);
                    float EnemyBaseProgress = GetFriendlyProgress(EnemyBaseFlag, MyTeamId);
                    ChosenGoal = (MyBaseProgress <= EnemyBaseProgress) ? MyBaseFlag : EnemyBaseFlag;
                }
                else if (!MyBaseFlag->IsCaptured())
                {
                    ChosenGoal = MyBaseFlag;
                }
                else
                {
                    ChosenGoal = EnemyBaseFlag;
                }
            }
        }
        else if (EnemyBaseFlag->IsCaptured())
        {
            ChosenGoal = !MidFlag->IsCaptured() ? MidFlag : MyBaseFlag;
        }
    }
    // ======================================================
    // POINT 1: No flags captured — default behavior
    // ======================================================
    else
    {
        bool bLockedToMid = BB->GetValueAsBool(LockedToMidZoneKeyName);
        float EnemyProgressOnBase = GetEnemyProgress(MyBaseFlag, MyTeamId);
        float EnemyProgressOnMid = GetEnemyProgress(MidFlag, MyTeamId);

        if (EnemyProgressOnBase >= TerritoryThreshold)
        {
            ChosenGoal = MyBaseFlag;
        }
        else if (bLockedToMid)
        {
            ChosenGoal = MidFlag;
        }
        else if (EnemyProgressOnMid >= TerritoryThreshold)
        {
            BB->SetValueAsBool(LockedToMidZoneKeyName, true);
            ChosenGoal = MidFlag;
        }
        else
        {
            ChosenGoal = MyBaseFlag;
        }
    }

    // ======================================================
    // SMART DISTRIBUTION: Check if the chosen goal is
    // overcrowded. If too many allies already heading there,
    // redirect to the next best flag.
    // ======================================================
    if (ChosenGoal)
    {
        int32 TotalAlive = GetTotalAliveAllies(MyTeamId);

        // Only apply distribution if we have enough skeletons alive in game 
        if (TotalAlive >= MinSkeletonsForDistribution)
        {
            // How many allies can go to one flag before it's "full"
            int32 MaxAtOneFlag = FMath::CeilToInt(TotalAlive * MaxPerFlagRatio);

            // Count how many allies are already assigned to our chosen goal
            int32 AlreadyAssigned = CountAlliesAssignedTo(ChosenGoal, MyTeamId, AIC);

            // If the flag is at capacity, redirect to secondary goal
            if (AlreadyAssigned >= MaxAtOneFlag)
            {
                AFlag* Secondary = GetSecondaryGoal(ChosenGoal, MyTeamId);
                if (Secondary)
                {
                    ChosenGoal = Secondary;
                }
            }
        }
    }

    // ======================================================
    // WRITE RESULT TO BLACKBOARD
    // ======================================================
    if (ChosenGoal)
    {
        BB->SetValueAsObject(GoalKeyName, ChosenGoal);
    }
}