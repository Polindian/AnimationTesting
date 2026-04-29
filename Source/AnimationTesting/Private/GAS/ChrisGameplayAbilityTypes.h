// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "ChrisGameplayAbilityTypes.generated.h"

UENUM(BlueprintType)
enum class EChrisAbilityInputID : uint8
{
	None				UMETA(DisplayName = "None"),
	BasicAttack			UMETA(DisplayName = "Basic Attack"),
	BasicAttack1		UMETA(DisplayName = "Basic Attack1"),
	BasicAttack2		UMETA(DisplayName = "Basic Attack2"),
	BasicAttack3		UMETA(DisplayName = "Basic Attack3"),
	BasicAttack4		UMETA(DisplayName = "Basic Attack4"),
	BasicAttack5		UMETA(DisplayName = "Basic Attack5"),
	BasicAttack6		UMETA(DisplayName = "Basic Attack6"),
	BasicAttack7		UMETA(DisplayName = "Basic Attack7"),
	Roll				UMETA(DisplayName = "Roll"),
	RollForward			UMETA(DisplayName = "RollForward"),
	RollForwardLeft		UMETA(DisplayName = "RollForwardLeft"),
	RollForwardRight	UMETA(DisplayName = "RollForwardRight"),
	RollLeft			UMETA(DisplayName = "RollLeft"),
	RollRight			UMETA(DisplayName = "RollRight"),
	RollBack			UMETA(DisplayName = "RollBack"),
	RollBackLeft		UMETA(DisplayName = "RollBackLeft"),
	RollBackRight		UMETA(DisplayName = "RollBackRight"),
	HeavyAttack			UMETA(DisplayName = "Heavy Attack"),
	HeavyAttack1		UMETA(DisplayName = "Heavy Attack 1"),
	HeavyAttack2		UMETA(DisplayName = "Heavy Attack 2"),
	HeavyAttack3Aim		UMETA(DisplayName = "Heavy Attack 3 Aim"),
	HeavyAttack3Shoot    UMETA(DisplayName = "Heavy Attack 3 Shoot"),
	EquipToggle			UMETA(DisplayName = "Equip Toggle"),
	Confirm				UMETA(DisplayName = "Confirm"),
	Cancel				UMETA(DisplayName = "Cancel")
};

USTRUCT(BlueprintType)
struct FGenericDamageEffectDefinition
{
	GENERATED_BODY()

public:
	FGenericDamageEffectDefinition();

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditAnywhere)
	FVector PushVelocity;
};

USTRUCT(BlueprintType)
struct FHeroBaseStats : public FTableRowBase
{
	GENERATED_BODY()
public:
	FHeroBaseStats();

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> Class;

	UPROPERTY(EditAnywhere)
	float BaseAttackDamage;

	UPROPERTY(EditAnywhere)
	float BaseArmor;

	UPROPERTY(EditAnywhere)
	float Intelligence;

	UPROPERTY(EditAnywhere)
	float IntelligenceGrowthRate;

	UPROPERTY(EditAnywhere)
	float Strength;
	
	UPROPERTY(EditAnywhere)
	float StrengthGrowthRate;

	UPROPERTY(EditAnywhere)
	float BaseMoveSpeed;

	UPROPERTY(EditAnywhere)
	float BaseMaxHealth;

	UPROPERTY(EditAnywhere)
	float BaseMaxMana;
};