// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Abilities/GameplayAbility.h"
#include "ChrisAbilitySystemStatics.generated.h"

class UGameplayAbility;
struct FGameplayAbilitySpec;
/**
 * 
 */
UCLASS()
class UChrisAbilitySystemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Basic Attack Tags
	static FGameplayTag GetBasicAttackAbilityTag();
	static FGameplayTag GetBasicAttack1AbilityTag();
	static FGameplayTag GetBasicAttack2AbilityTag();
	static FGameplayTag GetBasicAttack3AbilityTag();
	static FGameplayTag GetBasicAttack4AbilityTag();
	static FGameplayTag GetBasicAttack5AbilityTag();
	static FGameplayTag GetBasicAttack6AbilityTag();
	static FGameplayTag GetBasicAttack7AbilityTag();
	 
	// Heavy Attack Tags
	static FGameplayTag GetHeavyAttackAbilityTag();
	static FGameplayTag GetHeavyAttack1AbilityTag();
	static FGameplayTag GetHeavyAttack2AbilityTag();
	static FGameplayTag GetHeavyAttack3AbilityTag();

	// Basic Inputs
	static FGameplayTag GetBasicAttackInputPressedTag();
	static FGameplayTag GetBasicAttack1InputPressedTag();
	static FGameplayTag GetBasicAttack2InputPressedTag();
	static FGameplayTag GetBasicAttack3InputPressedTag();
	static FGameplayTag GetBasicAttack4InputPressedTag();
	static FGameplayTag GetBasicAttack5InputPressedTag();
	static FGameplayTag GetBasicAttack6InputPressedTag();
	static FGameplayTag GetBasicAttack7InputPressedTag();

	// Heavy Inputs
	static FGameplayTag GetHeavyAttackInputPressedTag();
	static FGameplayTag GetHeavyAttack1InputPressedTag();
	static FGameplayTag GetHeavyAttack2InputPressedTag();
	static FGameplayTag GetHeavyAttack3InputPressedTag();
	static FGameplayTag GetHeavyAttack3InputReleasedTag();
	static FGameplayTag GetHeavyAttack3ShootInputTag();
	
	// Status Tags
	static FGameplayTag GetDeadStatsTag();
	static FGameplayTag GetStunStatsTag();
	static FGameplayTag GetFallBackStatsTag();
	static FGameplayTag GetAimStatsTag();
	static FGameplayTag GetCrosshairTag();

	static FGameplayTag GetHealthFullStatsTag();
	static FGameplayTag GetHealthEmptyStatsTag();
	static FGameplayTag GetManaFullStatsTag();
	static FGameplayTag GetManaEmptyStatsTag();

	static FGameplayTag GetHeroRoleTag();
	static bool IsHero(const AActor* ActorToCheck);
	
	// Attribute Tags
	static FGameplayTag GetExperienceAttributeTag();
	static FGameplayTag GetSoulAttributeTag();

	// Equip Tags
	static FGameplayTag GetWeaponsEquippedTag();
	static FGameplayTag GetWeaponsUnequippedTag();

	// Upgrading Tags
	static bool IsAbilityAtMaxLevel(const FGameplayAbilitySpec& Spec);

	static float GetStaticCooldownDurationForAbility(const UGameplayAbility* Ability);
	static float GetStaticCostForAbility(const UGameplayAbility* Ability);

	static void SendLocalGameplayCue(AActor* TargetActor, const FHitResult& HitResult, const FGameplayTag& GameplayCueTag);
};