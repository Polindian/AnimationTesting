// Christopher Naglik All Rights Reserved


#include "Widgets/GameplayWidget.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/ChrisCharacter.h"
#include "Components/WidgetSwitcher.h"
#include "Components/CanvasPanel.h"
#include "Widgets/ValueGauge.h"
#include "Widgets/AbilityListView.h"
#include "GAS/ChrisAttributeSet.h"
#include "GAS/ChrisAbilitySystemComponent.h"
#include "GAS/CHeroAttributeSet.h"
#include "Widgets/StatsGauge.h"
#include "Widgets/GameplayMenu.h"

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

void UGameplayWidget::ResetAllCooldowns()
{
	if (AbilityListView)
	{
		AbilityListView->ResetAllCooldownVisuals();
	}
}

void UGameplayWidget::SetOwningPawnInputEnabled(bool bPawnInputEnabled)
{
	if(bPawnInputEnabled)
	{
		GetOwningPlayerPawn()->EnableInput(GetOwningPlayer());
	}
	else
	{
		GetOwningPlayerPawn()->DisableInput(GetOwningPlayer());
	}
}

void UGameplayWidget::SetFocusToGameAndUI()
{
	FInputModeGameAndUI GameAndUIInputMode;
	GameAndUIInputMode.SetHideCursorDuringCapture(false);
	GetOwningPlayer()->SetInputMode(GameAndUIInputMode);
}

void UGameplayWidget::SetFocusToGameOnly()
{
	FInputModeGameOnly GameOnlyInputMode;
	GetOwningPlayer()->SetInputMode(GameOnlyInputMode);
}
