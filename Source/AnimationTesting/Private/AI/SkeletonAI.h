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
	void SetGoal(AActor* Goal);
	
private:

	virtual void BeginPlay() override;

	void PickSkinBasedOnTeamID();

	virtual void OnRep_TeamID() override;

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
};
