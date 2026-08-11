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

	void ResetAllCooldowns();
	void ApplyHeavyAbilityCooldowns();

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

	/********************************/
	/*             Audio            */
	/********************************/

public:
	/** Called once after abilities are granted — watches every heavy ability's cooldown tags. */
	void BindSkillCooldownAudio();
	void StopLowHealthLoop();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


private:
	void SkillCooldownTagUpdated(const FGameplayTag Tag, int32 NewCount);

	UPROPERTY(Transient)
	TObjectPtr<class UAudioComponent> LowHealthAudio;

	// Shared by the vignette and the audio loop so they can't drift apart
	UPROPERTY(EditDefaultsOnly, Category = "Low Health")
	float LowHealthThreshold = 0.3f;

	// Above this the loop stops — the gap stops it stuttering around the line
	UPROPERTY(EditDefaultsOnly, Category = "Low Health")
	float LowHealthAudioStopThreshold = 0.35f;


};
