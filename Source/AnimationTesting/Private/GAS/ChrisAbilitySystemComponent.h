// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "GAS/ChrisGameplayAbilityTypes.h"
#include "ChrisAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class UChrisAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UChrisAbilitySystemComponent();
	void InitializeBaseAttributes();
	void ServerSideInit();
	void ApplyFullStatEffect();

	const TMap<EChrisAbilityInputID, TSubclassOf<UGameplayAbility>>& GetAbilities() const;
	bool IsAtMaxLevel() const;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_UpgradeAbilityWithID(EChrisAbilityInputID InputID);

	UFUNCTION(Client, Reliable)
	void Client_AbilitySpecLevelUpdated(FGameplayAbilitySpecHandle Handle, int NewLevel);


private:
	
	void ApplyInitialEffects();
	void GiveInitialAbilities();

	void AuthApplyGameEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int Level = 1);

	void HealthUpdated(const FOnAttributeChangeData& ChangeData);
	void ManaUpdated(const FOnAttributeChangeData& ChangeData);
	void ExperienceUpdated(const FOnAttributeChangeData& ChangeData);

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
	TMap<EChrisAbilityInputID, TSubclassOf<UGameplayAbility>> Abilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
	TMap<EChrisAbilityInputID, TSubclassOf<UGameplayAbility>> BasicAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
	class UPA_GenericAbilitySystem* AbilitySystemGenerics;
};
