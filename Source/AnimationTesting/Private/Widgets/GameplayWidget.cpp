// Christopher Naglik All Rights Reserved


#include "Widgets/GameplayWidget.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/ChrisCharacter.h"
#include "Widgets/ValueGauge.h"
#include "Widgets/AbilityListView.h"
#include "GAS/ChrisAttributeSet.h"
#include "GAS/ChrisAbilitySystemComponent.h"
#include "GAS/CHeroAttributeSet.h"
#include "Widgets/StatsGauge.h"

void UGameplayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	OwnerAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());

	if (OwnerAbilitySystemComponent)
	{
		HealthBar->SetAndBoundGameplayAttribute(OwnerAbilitySystemComponent,UChrisAttributeSet::GetHealthAttribute(),UChrisAttributeSet::GetMaxHealthAttribute());
		ManaBar->SetAndBoundGameplayAttribute(OwnerAbilitySystemComponent,UChrisAttributeSet::GetManaAttribute(),UChrisAttributeSet::GetMaxManaAttribute());

		AttackDamageGauge->BindToAttribute(OwnerAbilitySystemComponent, UChrisAttributeSet::GetAttackDamageAttribute());
		ArmorGauge->BindToAttribute(OwnerAbilitySystemComponent, UChrisAttributeSet::GetArmourAttribute());
		MoveSpeedGauge->BindToAttribute(OwnerAbilitySystemComponent, UChrisAttributeSet::GetMoveSpeedAttribute());
		IntelligenceGauge->BindToAttribute(OwnerAbilitySystemComponent, UCHeroAttributeSet::GetIntelligenceAttribute());
		StrengthGauge->BindToAttribute(OwnerAbilitySystemComponent, UCHeroAttributeSet::GetStrengthAttribute());
	}
}

void UGameplayWidget::ConfigureAbilities(const TMap<EChrisAbilityInputID, TSubclassOf<class UGameplayAbility>>& Abilities)
{
	AbilityListView->ConfigureAbilities(Abilities);
}

