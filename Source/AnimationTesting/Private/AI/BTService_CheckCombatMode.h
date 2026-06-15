// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "GameplayTagContainer.h"
#include "BTService_CheckCombatMode.generated.h"

class AFlag;
class UAIPerceptionComponent;

UCLASS()
class UBTService_CheckCombatMode : public UBTService
{
    GENERATED_BODY()

public:
    UBTService_CheckCombatMode();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
    // ======================================================
    // ZONE CONTROL
    // ======================================================

    // How much rate advantage (MyRate - EnemyRate) is needed to stay stationary
    UPROPERTY(EditAnywhere, Category = "Combat")
    float RateAdvantageThreshold = 4.f;

    // ======================================================
    // BLACKBOARD KEY NAMES
    // ======================================================

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FName ShouldMoveKeyName = "bShouldMoveToTarget";

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FName TargetKeyName = "Target";

    // Key for the dodge emergency flag (Bool)
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FName MustDodgeKeyName = "bMustDodge";

    // Key for the escape destination (Vector)
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FName DodgeLocationKeyName = "DodgeLocation";

    // ======================================================
    // AOE DODGE SETTINGS
    // ======================================================

    // Extra distance beyond the AoE radius that the skeleton
    // will flee to. Ensures they clear the blast zone completely..
    UPROPERTY(EditAnywhere, Category = "Combat|Dodge")
    float DodgeExtraDistance = 100.f;

    // Fallback radius to use if the enemy's SmashSweepRadius
    // and StunSweepRadius attributes are both 0 
    UPROPERTY(EditAnywhere, Category = "Combat|Dodge")
    float DefaultDangerRadius = 300.f;

    // Buffer distance BEYOND the blast radius where skeletons, stops advancing into radius
    UPROPERTY(EditAnywhere, Category = "Combat|Dodge")
    float DodgeBufferDistance = 250.f;

    // Speed override for dodge applied (instant, no ramp-up).
    UPROPERTY(EditAnywhere, Category = "Combat|Dodge")
    float DodgeSpeed = 600.f;

    // ======================================================
    // TARGET PRIORITY SCORING
    // ======================================================

    // How often (seconds) to fully re-evaluate target priority
    UPROPERTY(EditAnywhere, Category = "Combat|Targeting")
    float TargetReevalInterval = 2.0f;

    // Distance scoring: at 0 distance you get this many points,
    // at sight radius (1000) you get 0 points. Linear falloff.
    UPROPERTY(EditAnywhere, Category = "Combat|Targeting")
    float DistanceMaxScore = 500.f;

    // Flat bonus for targeting a player hero instead of an AI skeleton
    UPROPERTY(EditAnywhere, Category = "Combat|Targeting")
    float HeroBonus = 200.f;

    // Flat bonus if the enemy is standing inside a flag zone
    UPROPERTY(EditAnywhere, Category = "Combat|Targeting")
    float CapturingFlagBonus = 400.f;

    // Flat bonus if the enemy is below 25% health
    UPROPERTY(EditAnywhere, Category = "Combat|Targeting")
    float LowHealthBonus = 300.f;

    // Points per level of the enemy hero
    UPROPERTY(EditAnywhere, Category = "Combat|Targeting")
    float LevelWeight = 80.f;

    // Timer accumulator for periodic re-evaluation
    float TargetReevalTimer = 0.f;

    // ======================================================
    // INTERNAL STATE
    // ======================================================

    UPROPERTY()
    TArray<AFlag*> AllFlags;
    bool bFlagsCached = false;

    // Container holding all heavy attack ability tags.
    // Populated once in the constructor.
    FGameplayTagContainer HeavyAttackTags;

    void CacheFlags(UWorld* World);

    // Scores a potential target for priority selection
    float ScoreTarget(AActor* Enemy, const FVector& PawnLocation, uint8 MyTeamId) const;

    // Checks all perceived enemies for active AoE abilities.
    // Returns true if a dodge is needed, and fills OutDodgeLocation.
    bool CheckForAoEDanger(
        UAIPerceptionComponent* PerceptionComp,
        const FVector& PawnLocation,
        FVector& OutDodgeLocation) const;
};