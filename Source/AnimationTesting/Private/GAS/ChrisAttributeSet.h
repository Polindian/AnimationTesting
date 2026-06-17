// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "ChrisAttributeSet.generated.h"


#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
 GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
 GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
 GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
 GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class UChrisAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, Health)
	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, CachedHealthPercent)
	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, MaxHealth)
	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, Mana)
	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, CachedManaPercent)
	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, MaxMana)
	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, AttackDamage)
	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, Armour)
	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, MoveSpeed)
	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, MoveAcceleration)

	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, SmashSweepRadius)
	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, SmashDamageBonus)
	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, StunSweepRadius)
	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, ScorchedActive)
	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, DeadeyeActive)
	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, DeadeyeHitCounter)

	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, LightAttackBuffDamage)
	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, HeavyAttackBuffDamage)
	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, DamageReduction)
	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, DoubleDownChance)
	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, StackedOddsChance)
	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, LoadedDiceActive)
	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, LoadedDiceHitCounter)
	ATTRIBUTE_ACCESSORS	(UChrisAttributeSet, StasisWardReduction)
	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, HeavyDamageReduction)
	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, DominionBonus)
	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, StealthStrikeActive)

	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, ConsumableDamageBonus)
	ATTRIBUTE_ACCESSORS(UChrisAttributeSet, ConsumableDamageReduction)

	virtual void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	void RescaleHealth();
	void RescaleMana();

private:
	UPROPERTY(ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	UPROPERTY(ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	UPROPERTY(ReplicatedUsing = OnRep_Mana)
	FGameplayAttributeData Mana;

	UPROPERTY(ReplicatedUsing = OnRep_MaxMana)
	FGameplayAttributeData MaxMana;

	UPROPERTY(ReplicatedUsing = OnRep_AttackDamage)
	FGameplayAttributeData AttackDamage;

	UPROPERTY(ReplicatedUsing = OnRep_Armour)
	FGameplayAttributeData Armour;

	UPROPERTY(ReplicatedUsing = OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;

	UPROPERTY(ReplicatedUsing = OnRep_MoveAcceleration)
	FGameplayAttributeData MoveAcceleration;

	UPROPERTY()
	FGameplayAttributeData CachedHealthPercent;

	UPROPERTY()
	FGameplayAttributeData CachedManaPercent;


	// ********************************* //
	//         ABILITY UPGRADES          //
	//*********************************  //

	UPROPERTY(ReplicatedUsing = OnRep_SmashSweepRadius)
	FGameplayAttributeData SmashSweepRadius;

	UPROPERTY(ReplicatedUsing = OnRep_SmashDamageBonus)
	FGameplayAttributeData SmashDamageBonus;

	UPROPERTY(ReplicatedUsing = OnRep_StunSweepRadius)
	FGameplayAttributeData StunSweepRadius;

	UPROPERTY(ReplicatedUsing = OnRep_ScorchedActive)
	FGameplayAttributeData ScorchedActive;

	UPROPERTY(ReplicatedUsing = OnRep_DeadeyeActive)
	FGameplayAttributeData DeadeyeActive;

	UPROPERTY(ReplicatedUsing = OnRep_DeadeyeHitCounter)
	FGameplayAttributeData DeadeyeHitCounter;

	

	// ********************************* //
	//           SKILL BUFFS             //
	//*********************************  //

	UPROPERTY(ReplicatedUsing = OnRep_LightAttackBuffDamage)
	FGameplayAttributeData LightAttackBuffDamage;

	UPROPERTY(ReplicatedUsing = OnRep_HeavyAttackBuffDamage)
	FGameplayAttributeData HeavyAttackBuffDamage;

	UPROPERTY(ReplicatedUsing = OnRep_DamageReduction)
	FGameplayAttributeData DamageReduction;

	UPROPERTY(ReplicatedUsing = OnRep_DoubleDownChance)
	FGameplayAttributeData DoubleDownChance;

	UPROPERTY(ReplicatedUsing = OnRep_StackedOddsChance)
	FGameplayAttributeData StackedOddsChance;

	UPROPERTY(ReplicatedUsing = OnRep_LoadedDiceActive)
	FGameplayAttributeData LoadedDiceActive;

	UPROPERTY(ReplicatedUsing = OnRep_LoadedDiceHitCounter)
	FGameplayAttributeData LoadedDiceHitCounter;

	UPROPERTY(ReplicatedUsing = OnRep_StasisWardReduction)
	FGameplayAttributeData StasisWardReduction;

	UPROPERTY(ReplicatedUsing = OnRep_HeavyDamageReduction)
	FGameplayAttributeData HeavyDamageReduction;

	UPROPERTY(ReplicatedUsing = OnRep_DominionBonus)
	FGameplayAttributeData DominionBonus;

	UPROPERTY(ReplicatedUsing = OnRep_StealthStrikeActive)
	FGameplayAttributeData StealthStrikeActive;

	// ********************************* //
	//         CONSUMABLE BUFFS          //
	//*********************************  //

	UPROPERTY(ReplicatedUsing = OnRep_ConsumableDamageBonus)
	FGameplayAttributeData ConsumableDamageBonus;

	UPROPERTY(ReplicatedUsing = OnRep_ConsumableDamageReduction)
	FGameplayAttributeData ConsumableDamageReduction;

	

	// ********************************* //
	//           SKILL BUFFS             //
	//*********************************  //


	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_AttackDamage(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Armour(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MoveAcceleration(const FGameplayAttributeData& OldValue);

	// ********************************* //
	//         ABILITY UPGRADES          //
	//*********************************  //

	UFUNCTION()
	void OnRep_SmashSweepRadius(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_SmashDamageBonus(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_StunSweepRadius(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_ScorchedActive(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_DeadeyeActive(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_DeadeyeHitCounter(const FGameplayAttributeData& OldValue);
	

	// ********************************* //
	//           SKILL BUFFS             //
	//*********************************  //
	UFUNCTION()
	void OnRep_LightAttackBuffDamage(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_HeavyAttackBuffDamage(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_DamageReduction(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_DoubleDownChance(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_StackedOddsChance(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_LoadedDiceActive(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_LoadedDiceHitCounter(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_StasisWardReduction(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_HeavyDamageReduction(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_DominionBonus(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_StealthStrikeActive(const FGameplayAttributeData& OldValue);

	// ********************************* //
	//         CONSUMABLE BUFFS          //
	//*********************************  //

	UFUNCTION()
	void OnRep_ConsumableDamageBonus(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_ConsumableDamageReduction(const FGameplayAttributeData& OldValue);
};
