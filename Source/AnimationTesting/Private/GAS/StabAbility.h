// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GAS/ChrisGameplayAbility.h"
#include "GAS/ChrisGameplayAbilityTypes.h"
#include "StabAbility.generated.h"

/**
 * 
 */
UCLASS()
class UStabAbility : public UChrisGameplayAbility
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	UStabAbility();
private:
	UPROPERTY(EditDefaultsOnly, Category = "Stab")
	TSubclassOf<UGameplayEffect> StabDamageEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Combo")
	TMap<FName, FGenericDamageEffectDefinition> ComboDamageMap;

	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	float HitLaunchSpeed = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	FVector InitialHitPushVelocity;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* RightStabMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Targetting")
	float TargetSweepRadius = 35.f;

	static FGameplayTag GetRightStabTag();

	const FGenericDamageEffectDefinition* GetDamageEffectDefinitionForCurrentCombo() const;

	UFUNCTION()
	void StartCombo(FGameplayEventData EventData);

	/**********************************************/
	/*                Combo Changes               */
	/**********************************************/

	UFUNCTION()
	void HandleComboChangeEvent(FGameplayEventData EventData);

	UFUNCTION()
	void HandleComboCommit(FGameplayEventData EventData);

	UFUNCTION()
	void HandleComboDamageEvent(FGameplayEventData EventData);

	// Maps montage section name to the input tag required to advance FROM that section
	// Example: "combo01" -> BasicAttack1InputPressedTag means press left slash to go from combo01 to combo02
	UPROPERTY(EditDefaultsOnly, Category = "Combo")
	TMap<FName, FGameplayTag> ComboInputMap;

	FName NextComboName;
};
