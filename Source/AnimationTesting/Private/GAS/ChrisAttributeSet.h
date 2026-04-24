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
};
