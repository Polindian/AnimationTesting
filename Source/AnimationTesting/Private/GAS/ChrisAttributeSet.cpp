// Christopher Naglik All Rights Reserved


#include "GAS/ChrisAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

void UChrisAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
}

void UChrisAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0, GetMaxHealth()));
		SetCachedHealthPercent(GetHealth() / GetMaxHealth());
	}
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0, GetMaxMana()));
		SetCachedManaPercent(GetMana() / GetMaxMana());
	}
}

void UChrisAttributeSet::RescaleHealth()
{
	if (!GetOwningActor()->HasAuthority()) return;

	if(GetCachedHealthPercent() != 0 && GetHealth() != 0)
	{
		SetHealth(GetMaxHealth() * GetCachedHealthPercent());
	}
}

void UChrisAttributeSet::RescaleMana()
{
	if (!GetOwningActor()->HasAuthority()) return;

	if(GetCachedManaPercent() != 0 && GetMana() != 0)
	{
		SetMana(GetMaxMana() * GetCachedManaPercent());
	}
}

void UChrisAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, Health, OldValue);
}

void UChrisAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, MaxHealth, OldValue);
}
void UChrisAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, Mana, OldValue);
}
void UChrisAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, MaxMana, OldValue);
}

void UChrisAttributeSet::OnRep_AttackDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, AttackDamage, OldValue);
}

void UChrisAttributeSet::OnRep_Armour(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, Armour, OldValue);
}

void UChrisAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, MoveSpeed, OldValue);
}

void UChrisAttributeSet::OnRep_MoveAcceleration(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, MoveAcceleration, OldValue);
}


void UChrisAttributeSet::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const 
{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);
		DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, Health, COND_None, REPNOTIFY_Always);
		DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
		DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, Mana, COND_None, REPNOTIFY_Always);
		DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
		DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, AttackDamage, COND_None, REPNOTIFY_Always);
		DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, Armour, COND_None, REPNOTIFY_Always);
		DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
		DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, MoveAcceleration, COND_None, REPNOTIFY_Always);
}