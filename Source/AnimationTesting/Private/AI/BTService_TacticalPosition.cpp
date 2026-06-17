// Christopher Naglik All Rights Reserved

#include "AI/BTService_TacticalPosition.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Player/ChrisPlayerCharacter.h"
#include "Framework/Flag.h"
#include "EngineUtils.h"

UBTService_TacticalPosition::UBTService_TacticalPosition()
{
    NodeName = "Tactical Position";

    // Ticks every 0.3s — positions update smoothly
    Interval = 0.3f;
    RandomDeviation = 0.05f;
}

void UBTService_TacticalPosition::CacheFlags(UWorld* World)
{
    if (!World) return;

    AllFlags.Empty();
    for (TActorIterator<AFlag> It(World); It; ++It)
    {
        AllFlags.Add(*It);
    }

    bFlagsCached = (AllFlags.Num() > 0);
}

// ======================================================
// GET FLANK SLOT
// ======================================================
void UBTService_TacticalPosition::GetFlankSlot(AAIController* Self, AActor* Target, uint8 MyTeamId,int32& OutSlotIndex, int32& OutTotalCount) const
{
    // Collect all allied AI controllers targeting the same enemy
    TArray<AAIController*> AlliesOnSameTarget;

    UWorld* World = Self->GetWorld();
    if (!World)
    {
        OutSlotIndex = 0;
        OutTotalCount = 1;
        return;
    }

    for (TActorIterator<AAIController> It(World); It; ++It)
    {
        AAIController* OtherAIC = *It;
        if (!OtherAIC || !OtherAIC->GetPawn()) continue;
        if (OtherAIC->GetGenericTeamId().GetId() != MyTeamId) continue;

        // Check if this ally has the same target
        UBlackboardComponent* OtherBB = OtherAIC->GetBlackboardComponent();
        if (!OtherBB) continue;

        UObject* OtherTarget = OtherBB->GetValueAsObject(TargetKeyName);
        if (OtherTarget == Target)
        {
            AlliesOnSameTarget.Add(OtherAIC);
        }
    }

    // Sort by unique ID so the slot assignment is consistent
    // (same skeleton always gets the same slot)
    AlliesOnSameTarget.Sort([](const AAIController& A, const AAIController& B)
        {
            return A.GetUniqueID() < B.GetUniqueID();
        });

    OutTotalCount = AlliesOnSameTarget.Num();
    OutSlotIndex = AlliesOnSameTarget.IndexOfByKey(Self);

    // Safety: if we aren't in the list
    if (OutSlotIndex == INDEX_NONE)
    {
        OutSlotIndex = 0;
    }
}

// ======================================================
// GET SPREAD SLOT
// ======================================================
void UBTService_TacticalPosition::GetSpreadSlot(AAIController* Self, AFlag* Flag, uint8 MyTeamId,int32& OutSlotIndex, int32& OutTotalCount) const
{
    TArray<AAIController*> AlliesInZone;

    UWorld* World = Self->GetWorld();
    if (!World)
    {
        OutSlotIndex = 0;
        OutTotalCount = 1;
        return;
    }

    float FlagRadius = Flag->GetInfluenceRadius();
    FVector FlagLocation = Flag->GetActorLocation();

    for (TActorIterator<AAIController> It(World); It; ++It)
    {
        AAIController* OtherAIC = *It;
        if (!OtherAIC || !OtherAIC->GetPawn()) continue;
        if (OtherAIC->GetGenericTeamId().GetId() != MyTeamId) continue;

        float Dist = FVector::Dist(OtherAIC->GetPawn()->GetActorLocation(), FlagLocation);
        if (Dist <= FlagRadius)
        {
            AlliesInZone.Add(OtherAIC);
        }
    }

    AlliesInZone.Sort([](const AAIController& A, const AAIController& B)
        {
            return A.GetUniqueID() < B.GetUniqueID();
        });

    OutTotalCount = AlliesInZone.Num();
    OutSlotIndex = AlliesInZone.IndexOfByKey(Self);

    if (OutSlotIndex == INDEX_NONE)
    {
        OutSlotIndex = 0;
    }
}

// ======================================================
// GET CONSISTENT OFFSET
// ======================================================
FVector UBTService_TacticalPosition::GetConsistentOffset(APawn* Pawn, float MaxRadius) const
{
    // Use pawn's unique ID as a seed for consistent randomness
    uint32 Seed = Pawn->GetUniqueID();
    FRandomStream Stream(Seed);

    // Generate a random angle and distance
    float Angle = Stream.FRandRange(0.f, 360.f);
    float Distance = Stream.FRandRange(MaxRadius * 0.3f, MaxRadius);

    float Rad = FMath::DegreesToRadians(Angle);
    return FVector(FMath::Cos(Rad) * Distance, FMath::Sin(Rad) * Distance, 0.f);
}

void UBTService_TacticalPosition::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    // ======================================================
    // SETUP
    // ======================================================
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!AIC) return;

    APawn* Pawn = AIC->GetPawn();
    if (!Pawn) return;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return;

    if (!bFlagsCached)
    {
        CacheFlags(AIC->GetWorld());
    }

    uint8 MyTeamId = AIC->GetGenericTeamId().GetId();
    FVector PawnLocation = Pawn->GetActorLocation();

    AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetKeyName));

    // ======================================================
    // SPREAD POSITION (for Branch 3: moving to goal / in zone)
    // 
    // CASE A: Skeleton is INSIDE a flag zone (no enemies)
    //
    // CASE B: Skeleton is OUTSIDE the goal zone (traveling to it)
    // ======================================================
    AFlag* Goal = Cast<AFlag>(BB->GetValueAsObject(GoalKeyName));

    if (Goal && IsValid(Goal))
    {
        FVector FlagCenter = Goal->GetActorLocation();
        float FlagRadius = Goal->GetInfluenceRadius();
        float DistToFlag = FVector::Dist(PawnLocation, FlagCenter);

        if (DistToFlag <= FlagRadius)
        {
            // CASE A: Inside the zone — spread formation
            // Only spread if no target 
            if (!Target || !IsValid(Target))
            {
                int32 SlotIndex, TotalCount;
                GetSpreadSlot(AIC, Goal, MyTeamId, SlotIndex, TotalCount);

                // Distribute evenly in a circle around the flag center
                float AngleStep = 360.f / FMath::Max(TotalCount, 1);
                float Angle = SlotIndex * AngleStep;
                float Rad = FMath::DegreesToRadians(Angle);

                float SpreadDist = FlagRadius * SpreadRadiusRatio;
                FVector SpreadPos = FlagCenter + FVector(
                    FMath::Cos(Rad) * SpreadDist,
                    FMath::Sin(Rad) * SpreadDist,
                    0.f);

                BB->SetValueAsVector(SpreadPositionKeyName, SpreadPos);
            }
        }
        else
        {
            // CASE B: Outside the zone — path variety
            FVector Offset = GetConsistentOffset(Pawn, PathOffsetMax);
            FVector VariedDestination = FlagCenter + Offset;

            BB->SetValueAsVector(SpreadPositionKeyName, VariedDestination);
        }
    }
}