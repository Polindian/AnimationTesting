// Christopher Naglik All Rights Reserved


#include "GAS/ChrisAbilitySystemComponent.h"
#include "GAS/ChrisGameplayAbilityTypes.h"
#include "GAS/ChrisAttributeSet.h"
#include "GAS/CHeroAttributeSet.h"
#include "GAS/ChrisAbilitySystemStatics.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "GAS/PA_GenericAbilitySystem.h"

UChrisAbilitySystemComponent::UChrisAbilitySystemComponent()
{
	GetGameplayAttributeValueChangeDelegate(UChrisAttributeSet::GetHealthAttribute()).AddUObject(this, &UChrisAbilitySystemComponent::HealthUpdated);
	GetGameplayAttributeValueChangeDelegate(UChrisAttributeSet::GetManaAttribute()).AddUObject(this, &UChrisAbilitySystemComponent::ManaUpdated);
	GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetExperienceAttribute()).AddUObject(this, &UChrisAbilitySystemComponent::ExperienceUpdated);

	GenericConfirmInputID = (int32)EChrisAbilityInputID::Confirm;
	GenericCancelInputID = (int32)EChrisAbilityInputID::Cancel;
}

void UChrisAbilitySystemComponent::InitializeBaseAttributes()
{
	if (!AbilitySystemGenerics || !AbilitySystemGenerics->GetBaseStatsDataTable() || !GetOwner())
	{
		return;
	}

	const UDataTable* BaseStatsDataTable = AbilitySystemGenerics->GetBaseStatsDataTable();
	const FHeroBaseStats* BaseStats = nullptr;

	for(const TPair<FName, uint8*>& DataPair : BaseStatsDataTable->GetRowMap())
	{
		BaseStats = BaseStatsDataTable->FindRow<FHeroBaseStats>(DataPair.Key, "");
		if(BaseStats && BaseStats->Class == GetOwner()->GetClass())
		{
			break;
		}
	}

	if (BaseStats)
	{
		SetNumericAttributeBase(UChrisAttributeSet::GetMaxHealthAttribute(), BaseStats->BaseMaxHealth);
		SetNumericAttributeBase(UChrisAttributeSet::GetMaxManaAttribute(), BaseStats->BaseMaxMana);
		SetNumericAttributeBase(UChrisAttributeSet::GetAttackDamageAttribute(), BaseStats->BaseAttackDamage);
		SetNumericAttributeBase(UChrisAttributeSet::GetArmourAttribute(), BaseStats->BaseArmor);
		SetNumericAttributeBase(UChrisAttributeSet::GetMoveSpeedAttribute(), BaseStats->BaseMoveSpeed);

		SetNumericAttributeBase(UCHeroAttributeSet::GetIntelligenceAttribute(), BaseStats->Intelligence);
		SetNumericAttributeBase(UCHeroAttributeSet::GetIntelligenceGrowthRateAttribute(), BaseStats->IntelligenceGrowthRate);

		SetNumericAttributeBase(UCHeroAttributeSet::GetStrengthAttribute(), BaseStats->Strength);
		SetNumericAttributeBase(UCHeroAttributeSet::GetStrengthGrowthRateAttribute(), BaseStats->StrengthGrowthRate);
	}

	const FRealCurve* ExperienceCurve = AbilitySystemGenerics->GetExperienceCurve();
	if (ExperienceCurve)
	{
		int MaxLevel = ExperienceCurve->GetNumKeys();
		SetNumericAttributeBase(UCHeroAttributeSet::GetMaxLevelAttribute(), MaxLevel);

		float MaxExperience = ExperienceCurve->GetKeyValue(ExperienceCurve->GetLastKeyHandle());
		SetNumericAttributeBase(UCHeroAttributeSet::GetMaxLevelExperienceAttribute(), MaxExperience);

		UE_LOG(LogTemp, Log, TEXT("Max Level: %d, Max Experience: %f"), MaxLevel, MaxExperience);
	}

	ExperienceUpdated(FOnAttributeChangeData());
}

void UChrisAbilitySystemComponent::ServerSideInit()
{
	InitializeBaseAttributes();
	ApplyInitialEffects();
	GiveInitialAbilities();
}

void UChrisAbilitySystemComponent::ApplyInitialEffects()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	if (!AbilitySystemGenerics)
		return;
	for (const TSubclassOf<UGameplayEffect>& EffectClass: AbilitySystemGenerics->GetInitialEffects())
	{
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(EffectClass, 1, MakeEffectContext());
		ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
}

void UChrisAbilitySystemComponent::GiveInitialAbilities()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	
	for (const TPair<EChrisAbilityInputID, TSubclassOf<UGameplayAbility>>& AbilityPair : Abilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value, 0, (int32)AbilityPair.Key, nullptr));
	}
	for (const TPair<EChrisAbilityInputID, TSubclassOf<UGameplayAbility>>& AbilityPair : BasicAbilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value, 1, (int32)AbilityPair.Key, nullptr));
	}
	if (!AbilitySystemGenerics) return;
	for (const TSubclassOf<UGameplayAbility>& PassiveAbility : AbilitySystemGenerics->GetPassiveAbilities())
	{
		GiveAbility(FGameplayAbilitySpec(PassiveAbility, 1, -1, nullptr));
	}
}

void UChrisAbilitySystemComponent::ApplyFullStatEffect()
{
	if (!AbilitySystemGenerics)
		return;
	AuthApplyGameEffect(AbilitySystemGenerics->GetFullStatEffect());
}

const TMap<EChrisAbilityInputID, TSubclassOf<UGameplayAbility>>& UChrisAbilitySystemComponent::GetAbilities() const
{
	return Abilities;
}

bool UChrisAbilitySystemComponent::IsAtMaxLevel() const
{
	bool bFound;
	float CurrentLevel = GetGameplayAttributeValue(UCHeroAttributeSet::GetLevelAttribute(), bFound);
	float MaxLevel = GetGameplayAttributeValue(UCHeroAttributeSet::GetMaxLevelAttribute(), bFound);
	return CurrentLevel >= MaxLevel;
}

void UChrisAbilitySystemComponent::Server_UpgradeAbilityWithID_Implementation(EChrisAbilityInputID InputID)
{
	bool bFound = false;
	float UpgradePoint = GetGameplayAttributeValue(UCHeroAttributeSet::GetUpgradePointAttribute(), bFound);
	if (!bFound)
		return;

	FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromInputID(int32(InputID));
	if (!AbilitySpec || UChrisAbilitySystemStatics::IsAbilityAtMaxLevel(*AbilitySpec))
		return;

	SetNumericAttributeBase(UCHeroAttributeSet::GetUpgradePointAttribute(), UpgradePoint - 1);
	AbilitySpec->Level += 1;
	MarkAbilitySpecDirty(*AbilitySpec);

	Client_AbilitySpecLevelUpdated(AbilitySpec->Handle, AbilitySpec->Level);
}

bool UChrisAbilitySystemComponent::Server_UpgradeAbilityWithID_Validate(EChrisAbilityInputID InputID)
{
	return true;
}

void UChrisAbilitySystemComponent::Client_AbilitySpecLevelUpdated_Implementation(FGameplayAbilitySpecHandle Handle, int NewLevel)
{
	FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle);
	if (Spec)
	{
		Spec->Level = NewLevel;
		AbilitySpecDirtiedCallbacks.Broadcast(*Spec);
	}
}



void UChrisAbilitySystemComponent::AuthApplyGameEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int Level)
{
	
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(GameplayEffect, Level, MakeEffectContext());
		ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
}

void UChrisAbilitySystemComponent::HealthUpdated(const FOnAttributeChangeData& ChangeData)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	bool bFound = false;
	float MaxHealth = GetGameplayAttributeValue(UChrisAttributeSet::GetMaxHealthAttribute(), bFound);
	if (bFound && ChangeData.NewValue >= MaxHealth)
	{
		if (!HasMatchingGameplayTag(UChrisAbilitySystemStatics::GetHealthFullStatsTag()))
		{
			AddLooseGameplayTag(UChrisAbilitySystemStatics::GetHealthFullStatsTag());
		}
	}
	else
	{
		RemoveLooseGameplayTag(UChrisAbilitySystemStatics::GetHealthFullStatsTag());
	}

	if(ChangeData.NewValue <= 0)
	{
		if(!HasMatchingGameplayTag(UChrisAbilitySystemStatics::GetHealthEmptyStatsTag()))
		{
			AddLooseGameplayTag(UChrisAbilitySystemStatics::GetHealthEmptyStatsTag());

			if (AbilitySystemGenerics && AbilitySystemGenerics->GetDeathEffect())
			{
				AuthApplyGameEffect(AbilitySystemGenerics->GetDeathEffect());
			}
			FGameplayEventData DeadAbilityEventData;
			if (ChangeData.GEModData)
			DeadAbilityEventData.ContextHandle = ChangeData.GEModData->EffectSpec.GetContext();
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(), UChrisAbilitySystemStatics::GetDeadStatsTag(), DeadAbilityEventData);
		}
	}
	else
	{
		RemoveLooseGameplayTag(UChrisAbilitySystemStatics::GetHealthEmptyStatsTag());
	}
}

void UChrisAbilitySystemComponent::ManaUpdated(const FOnAttributeChangeData& ChangeData)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	bool bFound = false;
	float MaxMana = GetGameplayAttributeValue(UChrisAttributeSet::GetMaxManaAttribute(), bFound);
	if (bFound && ChangeData.NewValue >= MaxMana)
	{
		if (!HasMatchingGameplayTag(UChrisAbilitySystemStatics::GetManaFullStatsTag()))
		{
			AddLooseGameplayTag(UChrisAbilitySystemStatics::GetManaFullStatsTag());
		}
	}
	else
	{
		RemoveLooseGameplayTag(UChrisAbilitySystemStatics::GetManaFullStatsTag());
	}

	if (ChangeData.NewValue <= 0)
	{
		if (!HasMatchingGameplayTag(UChrisAbilitySystemStatics::GetManaEmptyStatsTag()))
		{
			AddLooseGameplayTag(UChrisAbilitySystemStatics::GetManaEmptyStatsTag());
		}
	}
	else
	{
		RemoveLooseGameplayTag(UChrisAbilitySystemStatics::GetManaEmptyStatsTag());
	}
}

void UChrisAbilitySystemComponent::ExperienceUpdated(const FOnAttributeChangeData& ChangeData)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	if (IsAtMaxLevel())
	{
		return;
	}

	if(!AbilitySystemGenerics)
	{
		return;
	}

	float CurrentExperience = ChangeData.NewValue;
	const FRealCurve* ExperienceCurve = AbilitySystemGenerics->GetExperienceCurve();
	if(!ExperienceCurve)
	{
		UE_LOG(LogTemp, Warning, TEXT("Experience curve data not found!"));
		return;
	}

	float PreviousLevelExperience = 0;
	float NextLevelExperience = 0;
	float NewLevel = 1;

	for(auto Iter = ExperienceCurve->GetKeyHandleIterator(); Iter; ++Iter)
	{
		float ExperienceToReachNextLevel = ExperienceCurve->GetKeyValue(*Iter);
		if (CurrentExperience < ExperienceToReachNextLevel)
		{
			NextLevelExperience = ExperienceToReachNextLevel;
			break;
		}

		PreviousLevelExperience = ExperienceToReachNextLevel;
		NewLevel = Iter.GetIndex() + 2;
	}

	float CurrentLevel = GetNumericAttributeBase(UCHeroAttributeSet::GetLevelAttribute());
	float CurrentUpgradePoint = GetNumericAttribute(UCHeroAttributeSet::GetUpgradePointAttribute());

	float LevelUpgraded = NewLevel - CurrentLevel;
	float NewUpgradePoint = CurrentUpgradePoint + LevelUpgraded;

	SetNumericAttributeBase(UCHeroAttributeSet::GetLevelAttribute(), NewLevel);
	SetNumericAttributeBase(UCHeroAttributeSet::GetPrevLevelExperienceAttribute(), PreviousLevelExperience);
	SetNumericAttributeBase(UCHeroAttributeSet::GetNextLevelExperienceAttribute(), NextLevelExperience);
	SetNumericAttributeBase(UCHeroAttributeSet::GetUpgradePointAttribute(), NewUpgradePoint);
}
