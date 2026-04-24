// Christopher Naglik All Rights Reserved


#include "AI/ChrisAIController.h"
#include "Character/ChrisCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/ChrisAbilitySystemComponent.h"
#include "GAS/ChrisAbilitySystemStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"




AChrisAIController::AChrisAIController()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("AI Perception Component");

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("Sight Config");

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = false;

	SightConfig->SightRadius = 1000.0f;
	SightConfig->LoseSightRadius = 1200.0f;
	SightConfig->SetMaxAge(5.0f);

	SightConfig->PeripheralVisionAngleDegrees = 180.0f;

	AIPerceptionComponent->ConfigureSense(*SightConfig);
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AChrisAIController::TargetPerceptionUpdated);
	AIPerceptionComponent->OnTargetPerceptionForgotten.AddDynamic(this, &AChrisAIController::TargetForgotten);
}

void AChrisAIController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);

	IGenericTeamAgentInterface* PawnTeamInterface = Cast<IGenericTeamAgentInterface>(NewPawn);
	if (PawnTeamInterface)
	{
		SetGenericTeamId(PawnTeamInterface->GetGenericTeamId());
		ClearAndDisableAllSenses();
		EnableAllSenses();
	}

	UAbilitySystemComponent* PawnASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(NewPawn);
	if (PawnASC)
	{
		PawnASC->RegisterGameplayTagEvent(UChrisAbilitySystemStatics::GetDeadStatsTag()).AddUObject(this, &AChrisAIController::PawnDeadTagUpdated);
		PawnASC->RegisterGameplayTagEvent(UChrisAbilitySystemStatics::GetStunStatsTag()).AddUObject(this, &AChrisAIController::PawnStunTagUpdated);
		PawnASC->RegisterGameplayTagEvent(UChrisAbilitySystemStatics::GetFallBackStatsTag()).AddUObject(this, &AChrisAIController::PawnFallBackTagUpdated);
	}
}

void AChrisAIController::BeginPlay()
{
	Super::BeginPlay();
	RunBehaviorTree(BehaviourTree);
}

void AChrisAIController::TargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus)
{
	if (Stimulus.WasSuccessfullySensed())
	{
		if (!GetCurrentTarget())
		{
			SetCurrentTarget(TargetActor);
		}
	}
	else
	{
		if (GetCurrentTarget() == TargetActor)
		{
			SetCurrentTarget(GetNextPerceivedActor());
		}
		ForgetActorIfDead(TargetActor);
	}
}

void AChrisAIController::TargetForgotten(AActor* ForgottenActor)
{
	if (!ForgottenActor) return;

	if(GetCurrentTarget() == ForgottenActor)
	{
		SetCurrentTarget(GetNextPerceivedActor());
	}
}

const UObject* AChrisAIController::GetCurrentTarget() const
{
	const UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	if (BlackboardComponent)
	{
		return GetBlackboardComponent()->GetValueAsObject(TargetBlackboardKeyName);
	}
	return nullptr;
}

void AChrisAIController::SetCurrentTarget(AActor* NewTarget)
{
	UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	if(!BlackboardComponent)
		return;
	
	if (NewTarget)
	{
		BlackboardComponent->SetValueAsObject(TargetBlackboardKeyName, NewTarget);
	}
	else
	{
		BlackboardComponent->ClearValue(TargetBlackboardKeyName);
	}
}

AActor* AChrisAIController::GetNextPerceivedActor() const
{
	if (PerceptionComponent)
	{
		TArray<AActor*> ActorsPerceived;
		AIPerceptionComponent->GetPerceivedHostileActors(ActorsPerceived);

		if (ActorsPerceived.Num() != 0)
		{
			return ActorsPerceived[0];
		}
	}

	return nullptr;
}

void AChrisAIController::ForgetActorIfDead(AActor* ActorToForget) const
{
	UAbilitySystemComponent* ActorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ActorToForget);
	if (!ActorASC) return;

	if(ActorASC->HasMatchingGameplayTag(UChrisAbilitySystemStatics::GetDeadStatsTag()))
	{
		for(UAIPerceptionComponent::TActorPerceptionContainer::TIterator Iter = AIPerceptionComponent->GetPerceptualDataIterator(); Iter; ++Iter)
		{
			if(Iter.Key() != ActorToForget)
			{
				continue;
			}

			for(FAIStimulus& Stimuli : Iter->Value.LastSensedStimuli)
			{
				Stimuli.SetStimulusAge(TNumericLimits<float>::Max());
			}
		}
	}
}

void AChrisAIController::ClearAndDisableAllSenses()
{
	AIPerceptionComponent->AgeStimuli(TNumericLimits<float>::Max());

	for (auto SenseConfigIt = AIPerceptionComponent->GetSensesConfigIterator(); SenseConfigIt; ++SenseConfigIt)
	{
		AIPerceptionComponent->SetSenseEnabled((*SenseConfigIt)->GetSenseImplementation(), false);
	}

	if(GetBlackboardComponent())
	{
		GetBlackboardComponent()->ClearValue(TargetBlackboardKeyName);
	}
}

void AChrisAIController::EnableAllSenses()
{
	for (auto SenseConfigIt = AIPerceptionComponent->GetSensesConfigIterator(); SenseConfigIt; ++SenseConfigIt)
	{
		AIPerceptionComponent->SetSenseEnabled((*SenseConfigIt)->GetSenseImplementation(), true);
	}
}

void AChrisAIController::PawnDeadTagUpdated(const FGameplayTag Tag, int32 Count)
{
	if (Count != 0)
	{
		GetBrainComponent()->StopLogic("Pawn is dead");
		ClearAndDisableAllSenses();
		bIsPawnDead = true;
	}
	else
	{
		GetBrainComponent()->StartLogic();
		EnableAllSenses();
		bIsPawnDead = false;
	}
}

void AChrisAIController::PawnStunTagUpdated(const FGameplayTag Tag, int32 Count)
{
	if (bIsPawnDead)
	{
		return;
	}
	if (Count != 0)
	{
		GetBrainComponent()->StopLogic("Stun");
	}
	else
	{
		GetBrainComponent()->StartLogic();
	}
}

void AChrisAIController::PawnFallBackTagUpdated(const FGameplayTag Tag, int32 Count)
{
	if (bIsPawnDead)
	{
		return;
	}
	if (Count != 0)
	{
		GetBrainComponent()->StopLogic("FallBack");
	}
	else
	{
		GetBrainComponent()->StartLogic();
	}
}
