// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayTagContainer.h"
#include "ChrisAIController.generated.h"

/**
 * 
 */
UCLASS()
class AChrisAIController : public AAIController
{
	GENERATED_BODY()

public:

	AChrisAIController();

	virtual void OnPossess(APawn* NewPawn) override;
	virtual void BeginPlay() override;

private:

	UPROPERTY(VisibleDefaultsOnly, Category = "Perception")
	class UAIPerceptionComponent* AIPerceptionComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = "Perception")
	class UAISenseConfig_Sight* SightConfig;

	UPROPERTY(EditDefaultsOnly, Category = "AI Behaviour")
	class UBehaviorTree* BehaviourTree;

	UPROPERTY(EditDefaultsOnly, Category = "AI Behaviour")
	FName TargetBlackboardKeyName = "Target";
	
	UFUNCTION()
	void TargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus);

	UFUNCTION()
	void TargetForgotten(AActor* ForgottenActor);

	const UObject* GetCurrentTarget() const;
	void SetCurrentTarget(AActor* NewTarget);

	AActor* GetNextPerceivedActor() const;

	void ForgetActorIfDead(AActor* ActorToForget) const;

	void ClearAndDisableAllSenses();
	void EnableAllSenses();

	void PawnDeadTagUpdated(const FGameplayTag Tag, int32 Count);
	void PawnStunTagUpdated(const FGameplayTag Tag, int32 Count);
	void PawnFallBackTagUpdated(const FGameplayTag Tag, int32 Count);

	bool bIsPawnDead = false;
};
