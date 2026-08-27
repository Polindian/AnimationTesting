// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GenericTeamAgentInterface.h"
#include "SkeletonBarrack.generated.h"

UCLASS()
class ASkeletonBarrack : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASkeletonBarrack();

	void StartSpawning(int32 PlayersPerTeam = 1);
	void StopSpawning();
	void DestroyAllSkeletons();

	// Game mode decides with barrack needs this
	FGenericTeamId GetBarrackTeamId() const { return BarrackTeamId; }

	// Practice arena: fixed group size regardless of player count
	void StartSpawningFixedGroup(int32 GroupSize);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere, Category = "Spawn")
	FGenericTeamId BarrackTeamId;

	UPROPERTY()
	TArray<class ASkeletonAI*> SkeletonPool;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<class ASkeletonAI> SkeletonClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TArray<class APlayerStart*> SpawnSpots;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	int SkeletonPerGroup = 1;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	int SkeletonsPerPlayer = 1;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	float GroupSpawnInterval = 30.f;

	bool IsSpawnSpotOccupied(const APlayerStart* SpawnSpot) const;

	// Must exceed twice the capsule radius, or spots read as free while a skeleton is still standing in them and the new one spawns interpenetrating
	UPROPERTY(EditAnywhere, Category = "Spawn")
	float OccupiedRadius = 120.f;

	int NextSpawnSpotIndex = -1;

	const APlayerStart* GetNextSpawnSpot();

	void SpawnNewGroup();
	void SpawnNewSkeletons(int Amount);

	ASkeletonAI* GetNextAvailableSkeleton(const TSet<class ASkeletonAI*>& Excluded) const;

	FTimerHandle SpawnIntervalTimerHnadle;

	FTransform SnapToGround(const FTransform& InTransform) const;
};
