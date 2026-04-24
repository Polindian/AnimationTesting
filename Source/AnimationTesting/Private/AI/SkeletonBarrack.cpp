// Christopher Naglik All Rights Reserved


#include "AI/SkeletonBarrack.h"
#include "GameFramework/PlayerStart.h"
#include "AI/SkeletonAI.h"

// Sets default values
ASkeletonBarrack::ASkeletonBarrack()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASkeletonBarrack::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority()) 
	{
		GetWorldTimerManager().SetTimer(SpawnIntervalTimerHnadle, this, &ASkeletonBarrack::SpawnNewGroup, GroupSpawnInterval, true);
	}
}

// Called every frame
void ASkeletonBarrack::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

const APlayerStart* ASkeletonBarrack::GetNextSpawnSpot()
{
	if (SpawnSpots.Num() == 0)
	{
		return nullptr;
	}
	
	++NextSpawnSpotIndex;

	if(NextSpawnSpotIndex >= SpawnSpots.Num())
	{
		NextSpawnSpotIndex = 0;
	}

	return SpawnSpots[NextSpawnSpotIndex];
}

void ASkeletonBarrack::SpawnNewGroup()
{
	int i = SkeletonPerGroup;

		while (i > 0)
		{
			FTransform SpawnTransform = GetActorTransform();
			if (const APlayerStart* NextSpawnSpot = GetNextSpawnSpot())
			{
				SpawnTransform = NextSpawnSpot->GetActorTransform();
			}

			ASkeletonAI* NextAvailableSkeleton = GetNextAvailableSkeleton();
			if (!NextAvailableSkeleton)
				break;

			NextAvailableSkeleton->SetActorTransform(SpawnTransform);
			NextAvailableSkeleton->Activate();
			--i;
		}

		SpawnNewSkeletons(i);
}

void ASkeletonBarrack::SpawnNewSkeletons(int Amount)
{
	for(int i = 0; i < Amount; i++)
	{
		FTransform SpawnTransform = GetActorTransform();
		if(const APlayerStart* NextSpawnSpot = GetNextSpawnSpot())
		{
			SpawnTransform = NextSpawnSpot->GetActorTransform();
		}

		ASkeletonAI* NewSkeleton = GetWorld()->SpawnActorDeferred<ASkeletonAI>(SkeletonClass, SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		NewSkeleton->SetGenericTeamId(BarrackTeamId);
		NewSkeleton->FinishSpawning(SpawnTransform);
		NewSkeleton->SetGoal(Goal);
		SkeletonPool.Add(NewSkeleton);
	}
}

ASkeletonAI* ASkeletonBarrack::GetNextAvailableSkeleton() const
{
	for (ASkeletonAI* Skeleton : SkeletonPool)
	{
		if (!Skeleton->IsActive())
		{
			return Skeleton;
		}
	}

	return nullptr;
}

