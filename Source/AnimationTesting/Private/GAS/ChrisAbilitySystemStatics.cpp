// Christopher Naglik All Rights Reserved


#include "GAS/ChrisAbilitySystemStatics.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"




/**********************************************/
/*                    Tags                    */
/**********************************************/

FGameplayTag UChrisAbilitySystemStatics::GetBasicAttackAbilityTag()
{
	return FGameplayTag::RequestGameplayTag("ability.basicattack");
}

FGameplayTag UChrisAbilitySystemStatics::GetBasicAttack1AbilityTag()
{
	return FGameplayTag::RequestGameplayTag("ability.basicattack1");
}

FGameplayTag UChrisAbilitySystemStatics::GetBasicAttack2AbilityTag()
{
	return FGameplayTag::RequestGameplayTag("ability.basicattack2");
}

FGameplayTag UChrisAbilitySystemStatics::GetBasicAttack3AbilityTag()
{
	return FGameplayTag::RequestGameplayTag("ability.basicattack3");
}

FGameplayTag UChrisAbilitySystemStatics::GetBasicAttack4AbilityTag()
{
	return FGameplayTag::RequestGameplayTag("ability.basicattack4");
}

FGameplayTag UChrisAbilitySystemStatics::GetBasicAttack5AbilityTag()
{
	return FGameplayTag::RequestGameplayTag("ability.basicattack5");
}

FGameplayTag UChrisAbilitySystemStatics::GetBasicAttack6AbilityTag()
{
	return FGameplayTag::RequestGameplayTag("ability.basicattack6");
}

FGameplayTag UChrisAbilitySystemStatics::GetBasicAttack7AbilityTag()
{
	return FGameplayTag::RequestGameplayTag("ability.basicattack7");
}

FGameplayTag UChrisAbilitySystemStatics::GetHeavyAttackAbilityTag()
{
	return FGameplayTag::RequestGameplayTag("ability.heavyattack");
}

FGameplayTag UChrisAbilitySystemStatics::GetHeavyAttack1AbilityTag()
{
	return FGameplayTag::RequestGameplayTag("ability.heavyattack1");
}

FGameplayTag UChrisAbilitySystemStatics::GetHeavyAttack2AbilityTag()
{
	return FGameplayTag::RequestGameplayTag("ability.heavyattack2");
}

FGameplayTag UChrisAbilitySystemStatics::GetHeavyAttack3AbilityTag()
{
	return FGameplayTag::RequestGameplayTag("ability.heavyattack3");
}

/**********************************************/
/*                    Input                   */
/**********************************************/

FGameplayTag UChrisAbilitySystemStatics::GetBasicAttackInputPressedTag()
{
	return FGameplayTag::RequestGameplayTag("ability.basicattack.pressed");
}

FGameplayTag UChrisAbilitySystemStatics::GetBasicAttack1InputPressedTag()
{
	return FGameplayTag::RequestGameplayTag("ability.basicattack1.pressed");
}

FGameplayTag UChrisAbilitySystemStatics::GetBasicAttack2InputPressedTag()
{
	return FGameplayTag::RequestGameplayTag("ability.basicattack2.pressed");
}

FGameplayTag UChrisAbilitySystemStatics::GetBasicAttack3InputPressedTag()
{
	return FGameplayTag::RequestGameplayTag("ability.basicattack3.pressed");
}

FGameplayTag UChrisAbilitySystemStatics::GetBasicAttack4InputPressedTag()
{
	return FGameplayTag::RequestGameplayTag("ability.basicattack4.pressed");
}

FGameplayTag UChrisAbilitySystemStatics::GetBasicAttack5InputPressedTag()
{
	return FGameplayTag::RequestGameplayTag("ability.basicattack5.pressed");
}

FGameplayTag UChrisAbilitySystemStatics::GetBasicAttack6InputPressedTag()
{
	return FGameplayTag::RequestGameplayTag("ability.basicattack6.pressed");
}

FGameplayTag UChrisAbilitySystemStatics::GetBasicAttack7InputPressedTag()
{
	return FGameplayTag::RequestGameplayTag("ability.basicattack7.pressed");
}

FGameplayTag UChrisAbilitySystemStatics::GetHeavyAttackInputPressedTag()
{
	return FGameplayTag::RequestGameplayTag("ability.heavyattack.pressed");
}

FGameplayTag UChrisAbilitySystemStatics::GetHeavyAttack1InputPressedTag()
{
	return FGameplayTag::RequestGameplayTag("ability.heavyattack1.pressed");
}

FGameplayTag UChrisAbilitySystemStatics::GetHeavyAttack2InputPressedTag()
{
	return FGameplayTag::RequestGameplayTag("ability.heavyattack2.pressed");
}

FGameplayTag UChrisAbilitySystemStatics::GetHeavyAttack3InputPressedTag()
{
	return FGameplayTag::RequestGameplayTag("ability.heavyattack3.pressed");
}

FGameplayTag UChrisAbilitySystemStatics::GetHeavyAttack3InputReleasedTag()
{
	return FGameplayTag::RequestGameplayTag("ability.heavyattack3.released");
}

FGameplayTag UChrisAbilitySystemStatics::GetHeavyAttack3ShootInputTag()
{
	return FGameplayTag::RequestGameplayTag("ability.heavyattack3.shoot");
}




FGameplayTag UChrisAbilitySystemStatics::GetDeadStatsTag()
{
	return FGameplayTag::RequestGameplayTag("stats.dead");
}

FGameplayTag UChrisAbilitySystemStatics::GetStunStatsTag()
{
	return FGameplayTag::RequestGameplayTag("stats.stun");
}

FGameplayTag UChrisAbilitySystemStatics::GetFallBackStatsTag()
{
	return FGameplayTag::RequestGameplayTag("stats.fallback");
}

FGameplayTag UChrisAbilitySystemStatics::GetAimStatsTag()
{
	return FGameplayTag::RequestGameplayTag("stats.aim");
}

FGameplayTag UChrisAbilitySystemStatics::GetCrosshairTag()
{
	return FGameplayTag::RequestGameplayTag("stats.crosshair");
}

FGameplayTag UChrisAbilitySystemStatics::GetHealthFullStatsTag()
{
	return FGameplayTag::RequestGameplayTag("stats.health.full");
}

FGameplayTag UChrisAbilitySystemStatics::GetHealthEmptyStatsTag()
{
	return FGameplayTag::RequestGameplayTag("stats.health.empty");
}

FGameplayTag UChrisAbilitySystemStatics::GetManaFullStatsTag()
{
	return FGameplayTag::RequestGameplayTag("stats.mana.full");
}

FGameplayTag UChrisAbilitySystemStatics::GetManaEmptyStatsTag()
{
	return FGameplayTag::RequestGameplayTag("stats.mana.empty");
}

FGameplayTag UChrisAbilitySystemStatics::GetHeroRoleTag()
{
	return FGameplayTag::RequestGameplayTag("role.hero");
}

bool UChrisAbilitySystemStatics::IsHero(const AActor* ActorToCheck)
{
	const IAbilitySystemInterface* ActorAbilitySystemInterface = Cast<IAbilitySystemInterface>(ActorToCheck);
	if(ActorAbilitySystemInterface)
	{
		UAbilitySystemComponent* ActorASC = ActorAbilitySystemInterface->GetAbilitySystemComponent();
		if(ActorASC)
		{
			return ActorASC->HasMatchingGameplayTag(GetHeroRoleTag());
		}
	}
	return false;
}

FGameplayTag UChrisAbilitySystemStatics::GetExperienceAttributeTag()
{
	return FGameplayTag::RequestGameplayTag("attr.experience");
}

FGameplayTag UChrisAbilitySystemStatics::GetSoulAttributeTag()
{
	return FGameplayTag::RequestGameplayTag("attr.soul");
}

FGameplayTag UChrisAbilitySystemStatics::GetWeaponsEquippedTag()
{
	return FGameplayTag::RequestGameplayTag("stats.weapon.equipped");
}

FGameplayTag UChrisAbilitySystemStatics::GetWeaponsUnequippedTag()
{
	return FGameplayTag::RequestGameplayTag("stats.weapon.unequipped");
}



bool UChrisAbilitySystemStatics::IsAbilityAtMaxLevel(const FGameplayAbilitySpec& Spec)
{
	return Spec.Level >= 4;
}

float UChrisAbilitySystemStatics::GetStaticCooldownDurationForAbility(const UGameplayAbility* Ability)
{
	if (!Ability)
		return 0.f;

	const UGameplayEffect* CooldownEffect = Ability->GetCooldownGameplayEffect();
	if (!CooldownEffect)
		return 0.f;

	float CooldownDuration = 0.f;

	CooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(1, CooldownDuration);
	return CooldownDuration;
}

float UChrisAbilitySystemStatics::GetStaticCostForAbility(const UGameplayAbility* Ability)
{
	if (!Ability)
		return 0.f;

	const UGameplayEffect* CostEffect = Ability->GetCostGameplayEffect();
	if (!CostEffect || CostEffect->Modifiers.Num() == 0)
		return 0.f;

	float Cost = 0.f;
	CostEffect->Modifiers[0].ModifierMagnitude.GetStaticMagnitudeIfPossible(1, Cost);
	return FMath::Abs(Cost);
}

void UChrisAbilitySystemStatics::SendLocalGameplayCue(AActor* TargetActor, const FHitResult& HitResult, const FGameplayTag& GameplayCueTag)
{
	/*
	if (!TargetActor || !GameplayCueTag.IsValid()) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (ASC)
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = HitResult.ImpactPoint;
		CueParams.Normal = HitResult.ImpactNormal;
		ASC->ExecuteGameplayCue(GameplayCueTag, CueParams);
	}*/

	if (!TargetActor || !GameplayCueTag.IsValid()) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (ASC)
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = HitResult.ImpactPoint;
		CueParams.Normal = HitResult.ImpactNormal;

		// Attach HitResult to EffectContext so Blueprint's GetHitResult works
		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		ContextHandle.AddHitResult(HitResult);
		CueParams.EffectContext = ContextHandle;

		ASC->ExecuteGameplayCue(GameplayCueTag, CueParams);
	}
}
