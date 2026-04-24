// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "GenericTeamAgentInterface.h"
#include "GameplayEffectTypes.h"
#include "ProjectileActor.generated.h"

UCLASS()
class AProjectileActor : public AActor, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectileActor();

	void ShootProjectile(float InSpeed, float InMaxDistance, const AActor* InTarget, FGenericTeamId InTeamId, FGameplayEffectSpecHandle InHitEffectHandle);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID);

	virtual FGenericTeamId GetGenericTeamId() const { return TeamId; }

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

private:
	//UPROPERTY(VisibleAnywhere)
	//USphereComponent* CollisionSphere;

	UPROPERTY(EditDefaultsOnly, Category = "GameplayCue")
	FGameplayTag HitGameplayCueTag;

	UPROPERTY(Replicated)
	FGenericTeamId TeamId;

	UPROPERTY(Replicated)
	FVector MoveDirection;

	UPROPERTY(Replicated)
	float ProjectileSpeed;

	UPROPERTY()
	const AActor* Target;

	FGameplayEffectSpecHandle HitEffectSpecHandle;
	FTimerHandle ShootTimerHandle;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void TravelMaxDistanceReached();
	void SendLocalGameplayCue(AActor* CueTargetActor, const FHitResult& HitResult);
};
