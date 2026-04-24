// Christopher Naglik All Rights Reserved


#include "AI/SkeletonAI.h"
#include "SkeletonAI.h"
#include "Components/PointLightComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

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
	RespawnImmediately();
}

void ASkeletonAI::SetGoal(AActor* Goal)
{
	if(AAIController* AIController = GetController<AAIController>())
	{
		if(UBlackboardComponent* BlackboardComponent = AIController->GetBlackboardComponent())
		{
			BlackboardComponent->SetValueAsObject(GoalBlackboardKeyName, Goal);
		}
	}
}

void ASkeletonAI::BeginPlay()
{
	Super::BeginPlay();
	TeamPointLight = FindComponentByClass<UPointLightComponent>();
	PickSkinBasedOnTeamID();
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
	PickSkinBasedOnTeamID();
}
