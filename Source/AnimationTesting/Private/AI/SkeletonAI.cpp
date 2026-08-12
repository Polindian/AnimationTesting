// Christopher Naglik All Rights Reserved


#include "AI/SkeletonAI.h"
#include "SkeletonAI.h"
#include "Components/PointLightComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Audio/ChrisAudioSubsystem.h"
#include "Audio/ChrisGameplayTags.h"
#include "GameFramework/PlayerController.h"
#include "Character/ChrisCharacter.h"


void ASkeletonAI::SetGenericTeamId(const FGenericTeamId& NewTeamId)
{
	Super::SetGenericTeamId(NewTeamId);
	PickSkinBasedOnTeamID();
}

bool ASkeletonAI::IsActive() const
{
	return !IsDead();
}

void ASkeletonAI::Activate()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	

	if (MoveComp)
	{
		MoveComp->SetMovementMode(MOVE_Walking);
		MoveComp->StopMovementImmediately();
		MoveComp->GravityScale = 1.0f;
	}

	// Re-enable collision in case it was disabled during death
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule)
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	// Re-enable mesh collision if it was changed for ragdoll
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetSimulatePhysics(false);
	GetMesh()->AttachToComponent(Capsule, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	RespawnImmediately();
}



void ASkeletonAI::BeginPlay()
{
	Super::BeginPlay();
	TeamPointLight = FindComponentByClass<UPointLightComponent>();
	PickSkinBasedOnTeamID();

	// Enable AI rotation: face the controller's focus point at all times
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 460.f, 0.f);

	if (GetNetMode() != NM_DedicatedServer)
	{
		ScheduleNextScream();
	}
}

void ASkeletonAI::PickSkinBasedOnTeamID()
{
	USkeletalMesh** Skin = SkinMap.Find(GetGenericTeamId());
	if (Skin)
	{
		GetMesh()->SetSkeletalMesh(*Skin);
	}


	UMaterialInterface** Material = MaterialMap.Find(GetGenericTeamId());
	if (Material)
	{
		GetMesh()->SetMaterial(0, *Material);  // Element 0
	}

	FLinearColor* LightColor = LightColorMap.Find(GetGenericTeamId());
	if (LightColor && TeamPointLight)
	{
		TeamPointLight->SetLightColor(*LightColor);
	}
}

void ASkeletonAI::OnRep_TeamID()
{
	Super::OnRep_TeamID();
	PickSkinBasedOnTeamID();
}

void ASkeletonAI::MoveSpeedUpdated(const FOnAttributeChangeData& Data)
{
	float ClampedSpeed = FMath::Clamp(Data.NewValue, 0.f, 700.f);
	GetCharacterMovement()->MaxWalkSpeed = ClampedSpeed;

	if (Data.NewValue > 700.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AI MoveSpeed] %s — clamped from %.0f to %.0f"),
			*GetName(), Data.NewValue, ClampedSpeed);
	}
}


void ASkeletonAI::OnDead()
{
	Super::OnDead();

	GetWorld()->GetTimerManager().ClearTimer(ScreamTimerHandle);

	if (GetNetMode() == NM_DedicatedServer) { return; }

	// The dead tag replicates, so this runs on every client — each one plays
	// the sound locally at this skeleton's position, no cue needed
	if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
	{
		Audio->PlayAtLocation(ChrisGameplayTags::Audio_World_Skeleton_Death, GetActorLocation());
		Audio->PlayAtLocation(ChrisGameplayTags::Audio_World_Skeleton_DeathScream, GetActorLocation());
	}
}

void ASkeletonAI::OnRespawn()
{
	Super::OnRespawn();

	if (GetNetMode() != NM_DedicatedServer)
	{
		ScheduleNextScream();
	}
}

void ASkeletonAI::ScheduleNextScream()
{
	// Randomised per skeleton so a batch spawned together doesn't scream in unison
	const float Delay = FMath::FRandRange(ScreamIntervalMin, ScreamIntervalMax);
	GetWorld()->GetTimerManager().SetTimer(ScreamTimerHandle, this, &ASkeletonAI::TryScream, Delay, false);
}

void ASkeletonAI::TryScream()
{
	if (!IsDead() && IsNearLocalPlayer(ScreamMaxDistance))
	{
		if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
		{
			Audio->PlayAtLocation(ChrisGameplayTags::Audio_World_Skeleton_Scream, GetActorLocation());
		}
	}

	// Reschedule either way, so a skeleton that wanders back into range resumes
	ScheduleNextScream();
}

bool ASkeletonAI::IsNearLocalPlayer(float MaxDistance) const
{
	const APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	const APawn* LocalPawn = PC ? PC->GetPawn() : nullptr;
	if (!LocalPawn) { return false; }

	// Squared distance avoids a square root on every skeleton every few seconds
	return FVector::DistSquared(GetActorLocation(), LocalPawn->GetActorLocation())
		<= FMath::Square(MaxDistance);
}