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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere, Category = "Spawn")
	FGenericTeamId BarrackTeamId;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	AActor* Goal;

	UPROPERTY()
	TArray<class ASkeletonAI*> SkeletonPool;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<class ASkeletonAI> SkeletonClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TArray<class APlayerStart*> SpawnSpots;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	int SkeletonPerGroup = 1;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	float GroupSpawnInterval = 30.f;

	int NextSpawnSpotIndex = -1;

	const APlayerStart* GetNextSpawnSpot();

	void SpawnNewGroup();
	void SpawnNewSkeletons(int Amount);

	ASkeletonAI* GetNextAvailableSkeleton() const;

	FTimerHandle SpawnIntervalTimerHnadle;


};
