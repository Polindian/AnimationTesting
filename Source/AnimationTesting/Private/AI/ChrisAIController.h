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

	// Activates the behavior tree so the AI begins acting.
	void StartAIBehavior();

	// Called by the game mode when the round ends.
	// Pauses the behavior tree so AI stops moving.
	void StopAIBehavior();

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

	FTimerHandle EquipTagTimerHandle;

	FTimerHandle LowHealthCheckTimerHandle;

	void CheckForLowHealthHeroes();

	UPROPERTY(EditDefaultsOnly, Category = "AI Targeting")
	float LowHealthThreshold = 0.25f;

	// Round-end stop is authoritative: the tag handlers below must not undo it
	// when a stun, knockback or death tag happens to clear afterwards
	bool bBehaviorStoppedForRound = false;
};
