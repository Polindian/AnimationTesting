// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Character/ChrisCharacter.h"
#include "SkeletonAI.generated.h"

/**
 * 
 */
UCLASS()
class ASkeletonAI : public AChrisCharacter
{
	GENERATED_BODY()
public:
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamId) override;

	bool IsActive() const;
	void Activate();
	//void SetGoal(AActor* Goal);
	
private:

	virtual void BeginPlay() override;

	void PickSkinBasedOnTeamID();

	virtual void OnRep_TeamID() override;

	virtual void MoveSpeedUpdated(const FOnAttributeChangeData& Data) override;

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	TMap<FGenericTeamId, USkeletalMesh*> SkinMap;

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	TMap<FGenericTeamId, FLinearColor> LightColorMap;

	UPROPERTY(BlueprintReadWrite, Category = "Visual", meta = (AllowPrivateAccess = "true"))
	class UPointLightComponent* TeamPointLight;

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	TMap<FGenericTeamId, UMaterialInterface*> MaterialMap;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName GoalBlackboardKeyName = "Goal";

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
	class UPA_GenericAbilitySystem* AbilitySystemGenerics;


/*********************************/
/*            Audio              */
/*********************************/
public:
	virtual void OnDead() override;
	virtual void OnRespawn() override;

private:
	void ScheduleNextScream();
	void TryScream();

	FTimerHandle ScreamTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	float ScreamIntervalMin = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	float ScreamIntervalMax = 5.f;

	// Skeletons further than this from the local player don't scream at all —
	// no point spending a voice on something inaudible
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	float ScreamMaxDistance = 4000.f;

	bool IsNearLocalPlayer(float MaxDistance) const;

};
