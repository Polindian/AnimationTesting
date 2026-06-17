// Christopher Naglik All Rights Reserved


#include "AI/SkeletonAI.h"
#include "SkeletonAI.h"
#include "Components/PointLightComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"


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

/*void ASkeletonAI::SetGoal(AActor* Goal)
{
	if(AAIController* AIController = GetController<AAIController>())
	{
		if(UBlackboardComponent* BlackboardComponent = AIController->GetBlackboardComponent())
		{
			BlackboardComponent->SetValueAsObject(GoalBlackboardKeyName, Goal);
		}
	}
}
*/


void ASkeletonAI::BeginPlay()
{
	Super::BeginPlay();
	TeamPointLight = FindComponentByClass<UPointLightComponent>();
	PickSkinBasedOnTeamID();

	// Enable AI rotation: face the controller's focus point at all times
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 460.f, 0.f);
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
