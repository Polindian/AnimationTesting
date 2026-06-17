// Christopher Naglik All Rights Reserved


#include "AI/SkeletonBarrack.h"
#include "GameFramework/PlayerStart.h"
#include "AI/SkeletonAI.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/ChrisAttributeSet.h"

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
	
}

// Called every frame
void ASkeletonBarrack::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ASkeletonBarrack::IsSpawnSpotOccupied(const APlayerStart* SpawnSpot) const
{
	if (!SpawnSpot) return false;

	FVector SpotLocation = SpawnSpot->GetActorLocation();

	for (ASkeletonAI* Skeleton : SkeletonPool)
	{
		if (!Skeleton || !Skeleton->IsActive()) continue;

		// Use 2D distance due to Z spawn offset
		float Distance = FVector::Dist2D(Skeleton->GetActorLocation(), SpotLocation);
		if (Distance < OccupiedRadius)
		{
			return true;
		}
	}

	return false;
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


const APlayerStart* ASkeletonBarrack::GetNextSpawnSpot()
{
	if (SpawnSpots.Num() == 0)
	{
		return nullptr;
	}

	// ======================================================
	// Skip any spot that has a living skeleton too close to it.
	// If ALL spots are occupied, fall back to the next one in
	// sequence anyway 
	// ======================================================
	int SpotsChecked = 0;
	while (SpotsChecked < SpawnSpots.Num())
	{
		++NextSpawnSpotIndex;
		if (NextSpawnSpotIndex >= SpawnSpots.Num())
		{
			NextSpawnSpotIndex = 0;
		}

		const APlayerStart* Candidate = SpawnSpots[NextSpawnSpotIndex];
		if (Candidate && !IsSpawnSpotOccupied(Candidate))
		{
			return Candidate;
		}

		++SpotsChecked;
	}

	// All spots occupied — just use the next one in sequence
	++NextSpawnSpotIndex;
	if (NextSpawnSpotIndex >= SpawnSpots.Num())
	{
		NextSpawnSpotIndex = 0;
	}
	return SpawnSpots[NextSpawnSpotIndex];
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

		// Snap to ground so the skeleton lands on the NavMesh
		SpawnTransform = SnapToGround(SpawnTransform);

		ASkeletonAI* NewSkeleton = GetWorld()->SpawnActorDeferred<ASkeletonAI>(SkeletonClass, SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		NewSkeleton->SetGenericTeamId(BarrackTeamId);
		NewSkeleton->FinishSpawning(SpawnTransform);
		SkeletonPool.Add(NewSkeleton);


		// ======================================================
		// Fill health & mana to max AFTER spawning.
		// ======================================================
		GetWorld()->GetTimerManager().SetTimerForNextTick([NewSkeleton]()
			{
				if (!NewSkeleton || !IsValid(NewSkeleton)) return;

				UAbilitySystemComponent* ASC =
					UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(NewSkeleton);
				if (ASC)
				{
					bool bFound = false;
					float MaxHP = ASC->GetGameplayAttributeValue(
						UChrisAttributeSet::GetMaxHealthAttribute(), bFound);

					if (bFound && MaxHP > 0.f)
					{
						ASC->SetNumericAttributeBase(
							UChrisAttributeSet::GetHealthAttribute(), MaxHP);

						bool bFoundMana = false;
						float MaxMP = ASC->GetGameplayAttributeValue(
							UChrisAttributeSet::GetMaxManaAttribute(), bFoundMana);
						if (bFoundMana && MaxMP > 0.f)
						{
							ASC->SetNumericAttributeBase(
								UChrisAttributeSet::GetManaAttribute(), MaxMP);
						}

						UE_LOG(LogTemp, Warning, TEXT("[SpawnNew] %s — Health filled to %.1f"),
							*NewSkeleton->GetName(), MaxHP);
					}
				}
			});
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

void ASkeletonBarrack::StopSpawning()
{
	GetWorldTimerManager().ClearTimer(SpawnIntervalTimerHnadle);
}

void ASkeletonBarrack::StartSpawning(int32 PlayersPerTeam)
{
	SkeletonPerGroup = PlayersPerTeam * SkeletonsPerPlayer;

	SpawnNewGroup();

	GetWorldTimerManager().SetTimer(
		SpawnIntervalTimerHnadle, this,
		&ASkeletonBarrack::SpawnNewGroup, GroupSpawnInterval, true);
}

void ASkeletonBarrack::DestroyAllSkeletons()
{
	GetWorldTimerManager().ClearTimer(SpawnIntervalTimerHnadle);
	for (ASkeletonAI* Skeleton : SkeletonPool)
	{
		if (Skeleton)
		{
			Skeleton->Destroy();
		}
	}
	SkeletonPool.Empty();
}

FTransform ASkeletonBarrack::SnapToGround(const FTransform& InTransform) const
{
	FTransform Result = InTransform;
	FVector Location = InTransform.GetLocation();

	// Trace straight down from the spawn point to find the floor.
	FHitResult GroundHit;
	FVector TraceStart = Location;
	FVector TraceEnd = Location - FVector(0.f, 0.f, 1000.f);

	if (GetWorld()->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_WorldStatic))
	{
		Location.Z = GroundHit.ImpactPoint.Z + 90.f;
		Result.SetLocation(Location);
	}

	return Result;
}

