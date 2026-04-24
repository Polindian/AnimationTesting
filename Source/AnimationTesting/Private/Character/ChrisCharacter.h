// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemInterface.h"
#include "GAS/ChrisGameplayAbilityTypes.h" 
#include "Abilities/GameplayAbility.h"
#include "GenericTeamAgentInterface.h"
#include "GameplayEffectTypes.h"
#include "ChrisCharacter.generated.h"


UCLASS()
class AChrisCharacter : public ACharacter, public IAbilitySystemInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AChrisCharacter();
	void ServerSideInit();
	void ClientSideInit();
	bool IsLocallyControlledByPlayer() const;

	// Only called on the server
	virtual void PossessedBy(AController* NewController) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	const TMap<EChrisAbilityInputID, TSubclassOf<UGameplayAbility>>& GetAbilities() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;



	/********************************/
	/*        GameplayAbility       */
	/********************************/

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendGameplayEventToSelf(const FGameplayTag& EventTag, const FGameplayEventData& EventData);

protected:
	void UpgradeAbilityWithInputID(EChrisAbilityInputID InputID);

private:
	void BindGASChangeDelegate();
	void DeathTagUpdated(const FGameplayTag Tag, int32 NewCount);
	void StunTagUpdated(const FGameplayTag Tag, int32 NewCount);
	void FallBackTagUpdated(const FGameplayTag Tag, int32 NewCount);

	void AimTagUpdated(const FGameplayTag Tag, int32 NewCount);
	void SetIsAiming(bool bIsAiming);
	virtual void OnAimStateChanged(bool bIsAiming);

	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay Ability")
	class UChrisAbilitySystemComponent* ChrisAbilitySystemComponent;
	UPROPERTY()
	class UChrisAttributeSet* ChrisAttributeSet;

	void MoveSpeedUpdated(const FOnAttributeChangeData& Data);
	void MoveAccelerationUpdated(const FOnAttributeChangeData& Data);

	void MaxHealthUpdated(const FOnAttributeChangeData& Data);
	void MaxManaUpdated(const FOnAttributeChangeData& Data);


	/********************************/
	/*				UI				*/
	/********************************/
private:
	UPROPERTY(VisibleDefaultsOnly, Category = "UI")
	class UWidgetComponent* OverheadWidgetComponent;
	void ConfigureOverheadStatusWidget();

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	float HeadStatGaugeVisibilityCheckUpdateGap = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	float HeadStatGaugeVisibilityCheckUpdateRangeSquared = 10000000.f;

	FTimerHandle HeadStatGaugeVisibilityUpdateTimerHandle;

	void UpdateHeadStatGaugeVisibility();

	void SetStatusGaugeEnabled(bool bIsEnabled);

	/********************************/
	/*            Stun              */
	/********************************/

private:
	UPROPERTY(EditDefaultsOnly, Category = "Stun")
	UAnimMontage* StunMontage;

	virtual void OnStun();
	virtual void OnRecoverFromStun();

	UPROPERTY(EditDefaultsOnly, Category = "FallBack")
	UAnimMontage* FallBackMontage;

	virtual void OnFallBack();
	virtual void OnRecoverFromFallBack();

	/********************************/
	/*        Death & Respawn       */
	/********************************/
public:

	bool IsDead() const;
	void RespawnImmediately();
private:

	FTransform MeshRelativeTransform;

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	UAnimMontage* DeathMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeathMontageFinishTimeShift = -0.8f;

	FTimerHandle DeathMontageTimerHandle;

	void DeathMontageFinished();
	void SetRagdollEnabled(bool bIsEnabled);


	void PlayDeathAnimation();

	void StartDeathSequence();
	void Respawn();

	virtual void OnDead();
	virtual void OnRespawn();


	/********************************/
	/*            Team              */
	/********************************/
public:
	/** Assigns Team Agent to given TeamID */
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

	/** Retrieve team identifier in form of FGenericTeamId */
	virtual FGenericTeamId GetGenericTeamId() const override;

private:
	UPROPERTY(ReplicatedUsing = OnRep_TeamID)
	FGenericTeamId TeamID;

	UFUNCTION()
	virtual void OnRep_TeamID();


	/********************************/
	/*               AI             */
	/********************************/

	private:

		void SetAIPerceptionStimuliSourceEnabled(bool bIsEnabled);
		UPROPERTY()
		class UAIPerceptionStimuliSourceComponent* PerceptionStimuliSourceComponent;
};
