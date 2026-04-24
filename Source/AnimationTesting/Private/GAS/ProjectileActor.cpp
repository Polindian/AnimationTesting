// Christopher Naglik All Rights Reserved


#include "GAS/ProjectileActor.h"
#include "GAS/ChrisAbilitySystemStatics.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"

// Sets default values
AProjectileActor::AProjectileActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));
	SetRootComponent(RootComp);


	bReplicates = true;
}

void AProjectileActor::ShootProjectile(float InSpeed, float InMaxDistance, const AActor* InTarget, FGenericTeamId InTeamId, FGameplayEffectSpecHandle InHitEffectHandle)
{
	Target = InTarget;
	ProjectileSpeed = InSpeed;

	FRotator OwnerViewRotation = GetActorRotation();

	SetGenericTeamId(InTeamId);

	if (GetOwner())
	{
		FVector OwnerViewLocation;
		GetOwner()->GetActorEyesViewPoint(OwnerViewLocation, OwnerViewRotation);
	}

	MoveDirection = OwnerViewRotation.Vector();

	HitEffectSpecHandle = InHitEffectHandle;

	float TravelMaxTime = InMaxDistance / InSpeed;
	GetWorldTimerManager().SetTimer(ShootTimerHandle, this, &AProjectileActor::TravelMaxDistanceReached, TravelMaxTime);
}

void AProjectileActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AProjectileActor, MoveDirection);
	DOREPLIFETIME(AProjectileActor, TeamId);
	DOREPLIFETIME(AProjectileActor, ProjectileSpeed);
}

void AProjectileActor::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamId = NewTeamID;
}

/*void AProjectileActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	if(!OtherActor || OtherActor == GetOwner())
	{
		return;
	}

	if(GetTeamAttitudeTowards(*OtherActor) != ETeamAttitude::Hostile)
	{
		return;
	}

	UAbilitySystemComponent* OtherASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (IsValid(OtherASC))
	{
		if(HasAuthority() && HitEffectSpecHandle.IsValid())
		{
			OtherASC->ApplyGameplayEffectSpecToSelf(*HitEffectSpecHandle.Data.Get());
			GetWorldTimerManager().ClearTimer(ShootTimerHandle);
		}

		FHitResult HitResult;
		HitResult.ImpactPoint = GetActorLocation();
		HitResult.ImpactNormal = GetActorForwardVector();

		SendLocalGameplayCue(OtherActor, HitResult);

		Destroy();
	}
}
*/

void AProjectileActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	if (!OtherActor || OtherActor == GetOwner())
	{
		return;
	}

	if (GetTeamAttitudeTowards(*OtherActor) != ETeamAttitude::Hostile)
	{
		return;
	}

	UAbilitySystemComponent* OtherASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (IsValid(OtherASC))
	{
		if (HasAuthority() && HitEffectSpecHandle.IsValid())
		{
			OtherASC->ApplyGameplayEffectSpecToSelf(*HitEffectSpecHandle.Data.Get());
			GetWorldTimerManager().ClearTimer(ShootTimerHandle);

			UE_LOG(LogTemp, Warning, TEXT("Projectile: Applied GE to %s"), *OtherActor->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Projectile: HasAuthority=%d, HitEffectValid=%d"), HasAuthority(), HitEffectSpecHandle.IsValid());
		}

		FHitResult HitResult;
		HitResult.Location = GetActorLocation();
		HitResult.ImpactPoint = GetActorLocation();
		HitResult.ImpactNormal = GetActorForwardVector();

		UChrisAbilitySystemStatics::SendLocalGameplayCue(OtherActor, HitResult, HitGameplayCueTag);

		Destroy();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Projectile: OtherActor %s has NO ASC!"), *OtherActor->GetName());
	}
}

// Called when the game starts or when spawned
void AProjectileActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProjectileActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		if (Target)
		{
			MoveDirection = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		}
	}

	SetActorLocation(GetActorLocation() + MoveDirection * ProjectileSpeed * DeltaTime);
}

void AProjectileActor::TravelMaxDistanceReached()
{
	Destroy();
}

void AProjectileActor::SendLocalGameplayCue(AActor* CueTargetActor, const FHitResult& HitResult)
{
	FGameplayCueParameters CueParams;
	CueParams.Location = HitResult.ImpactPoint;
	CueParams.Normal = HitResult.ImpactNormal;

	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(CueTargetActor, HitGameplayCueTag, EGameplayCueEvent::Executed, CueParams);
}

