// Christopher Naglik All Rights Reserved


#include "AI/ChrisAIController.h"
#include "Character/ChrisCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/ChrisAbilitySystemComponent.h"
#include "GAS/ChrisAbilitySystemStatics.h"
#include "GAS/ChrisAttributeSet.h"
#include "Player/ChrisPlayerCharacter.h"
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
	SightConfig->LoseSightRadius = 2000.0f;
	SightConfig->SetMaxAge(5.0f);

	SightConfig->PeripheralVisionAngleDegrees = 350.0f;

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

	// ======================================================
	// Force the WeaponsEquipped tag after a short delay.
	// SwordEquipComponent::BeginPlay runs during spawn and
	// calls UpdateEquippedTag which REMOVES the equipped tag
	// (because it defaults to Unequipped state). A next-tick
	// timer still loses to late BeginPlay calls. A 0.5s delay
	// guarantees everything has fully initialized.
	//
	// Uses the correct tag from ChrisAbilitySystemStatics
	// (NOT a raw string) so it matches the registered tag.
	// ======================================================
	TWeakObjectPtr<APawn> WeakPawn = NewPawn;
	GetWorld()->GetTimerManager().SetTimer(EquipTagTimerHandle, [WeakPawn]()
		{
			if (!WeakPawn.IsValid()) return;

			UAbilitySystemComponent* ASC =
				UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(WeakPawn.Get());
			if (ASC)
			{
				ASC->AddLooseGameplayTag(UChrisAbilitySystemStatics::GetWeaponsEquippedTag());
				ASC->RemoveLooseGameplayTag(UChrisAbilitySystemStatics::GetWeaponsUnequippedTag());

				bool bHasTag = ASC->HasMatchingGameplayTag(UChrisAbilitySystemStatics::GetWeaponsEquippedTag());
				UE_LOG(LogTemp, Warning, TEXT("[AI] WeaponsEquipped forced — has tag: %d"), bHasTag);
			}
		}, 0.5f, false);

	GetWorld()->GetTimerManager().SetTimer(LowHealthCheckTimerHandle, this,&AChrisAIController::CheckForLowHealthHeroes, 0.5f, true);
}

void AChrisAIController::BeginPlay()
{
	Super::BeginPlay();
	
	RunBehaviorTree(BehaviourTree);
}

void AChrisAIController::StartAIBehavior()
{
	bBehaviorStoppedForRound = false;

	if (GetBrainComponent())
	{
		GetBrainComponent()->StartLogic();
	}
}

void AChrisAIController::StopAIBehavior()
{
	bBehaviorStoppedForRound = true;

	if (GetBrainComponent())
	{
		GetBrainComponent()->StopLogic("RoundEnded");
		GetWorld()->GetTimerManager().ClearTimer(LowHealthCheckTimerHandle);
	}

	// Otherwise they keep swinging at whatever they were already targeting
	if (APawn* MyPawn = GetPawn())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MyPawn))
		{
			ASC->CancelAllAbilities();
		}
	}

	StopMovement();
	ClearAndDisableAllSenses();
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
		// Don't drop target immediately — let MaxAge expiration handle it
		// via TargetForgotten. Only forget dead actors instantly.
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
	if (AIPerceptionComponent)
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
		return;
	}

	if (GetBlackboardComponent())
	{
		GetBlackboardComponent()->SetValueAsBool("bLockedToMidZone", false);
	}

	// Tracked even while the brain stays stopped, or the next round starts
	// with this AI still believed to be dead
	bIsPawnDead = false;

	// Pooled respawns clear the dead tag, so without this every skeleton the
	// barrack recycles would restart its brain mid round-end
	if (bBehaviorStoppedForRound) { return; }

	GetBrainComponent()->StartLogic();
	EnableAllSenses();
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
		return;
	}

	// A stun that expires after the round ended must not wake the brain back up
	if (bBehaviorStoppedForRound) { return; }

	GetBrainComponent()->StartLogic();
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
		return;
	}

	if (bBehaviorStoppedForRound) { return; }

	GetBrainComponent()->StartLogic();
}

void AChrisAIController::CheckForLowHealthHeroes()
{
	if (!GetPawn()) return;
	if (bIsPawnDead) return;

	TArray<AActor*> PerceivedEnemies;
	AIPerceptionComponent->GetPerceivedHostileActors(PerceivedEnemies);

	AActor* BestTarget = nullptr;
	float BestDist = TNumericLimits<float>::Max();

	for (AActor* Enemy : PerceivedEnemies)
	{
		if (!Enemy || !IsValid(Enemy)) continue;

		// Only heroes, not AI
		if (!Cast<AChrisPlayerCharacter>(Enemy)) continue;
		// Skip dead
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Enemy))
		{
			UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
			if (!ASC) continue;

			bool bF1, bF2;
			float HP = ASC->GetGameplayAttributeValue(UChrisAttributeSet::GetHealthAttribute(), bF1);
			float MaxHP = ASC->GetGameplayAttributeValue(UChrisAttributeSet::GetMaxHealthAttribute(), bF2);

			if (bF1 && bF2 && MaxHP > 0.f && (HP / MaxHP) < LowHealthThreshold)
			{
				float Dist = FVector::Dist(GetPawn()->GetActorLocation(), Enemy->GetActorLocation());
				if (Dist < BestDist)
				{
					BestDist = Dist;
					BestTarget = Enemy;
				}
			}
		}
	}

	if (BestTarget)
	{
		SetCurrentTarget(BestTarget); // override target
	}
}
