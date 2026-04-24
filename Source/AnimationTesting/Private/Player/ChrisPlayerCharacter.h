// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Character/ChrisCharacter.h"
#include "InputActionValue.h"
#include "GAS/ChrisGameplayAbilityTypes.h"
#include "ChrisPlayerCharacter.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class AChrisPlayerCharacter : public AChrisCharacter
{
	GENERATED_BODY()

public:
	AChrisPlayerCharacter();
	virtual void PawnClientRestart() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(VisibleDefaultsOnly, Category = "View")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleDefaultsOnly, Category = "View")
	class UCameraComponent* ViewCam;

	FVector GetLookRightDir() const;
	FVector GetLookFwdDir() const;
	FVector GetMoveFwdDir() const;

	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
	class USwordEquipComponent* SwordEquipComponent;

	/**********************************************/
	/*                Input Setup                 */
	/**********************************************/
private:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* JumpInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* LookInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* MoveInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	class UInputAction* UpgradeAbilityLeaderAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TMap<EChrisAbilityInputID, class UInputAction*> GameplayAbilityInputActions;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* GameplayInputMappingContext;

	// NEW: Attack Input Actions (one for each attack)
	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	class UInputAction* IA_BasicAttack;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	class UInputAction* IA_BasicAttack1;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	class UInputAction* IA_BasicAttack2;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	class UInputAction* IA_BasicAttack3;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	class UInputAction* IA_BasicAttack4;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	class UInputAction* IA_BasicAttack5;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	class UInputAction* IA_BasicAttack6;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	class UInputAction* IA_BasicAttack7;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")

	class UInputAction* IA_Roll;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")

	class UInputAction* IA_HeavyAttack;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	class UInputAction* IA_HeavyAttack1;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	class UInputAction* IA_HeavyAttack2;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	class UInputAction* IA_HeavyAttack3;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	class UInputAction* IA_HeavyAttack3Shoot;

	


	void HandleLookInput(const FInputActionValue& InputActionValue);
	void HandleMoveInput(const FInputActionValue& InputActionValue);
	void HandleAbilityInput(const FInputActionValue& InputActionValue, EChrisAbilityInputID InputID);
	bool bIsUpgradeAbilityLeaderDown = false;
	void UpgradeAbilityLeaderDown(const FInputActionValue& InputActionValue);
	void UpgradeAbilityLeaderUp(const FInputActionValue& InputActionValue);

	EChrisAbilityInputID GetRollDirectionFromInput(FVector2D MoveInput) const;


	void SetInputEnabledFromPlayerController(bool bEnabled);

	/**********************************************/
	/*               Death & Respawn              */
	/**********************************************/

	virtual void OnDead() override;
	virtual void OnRespawn() override;

	/**********************************************/
	/*                   Stun                     */
	/**********************************************/

	virtual void OnStun() override;
	virtual void OnRecoverFromStun() override;

	virtual void OnFallBack() override;
	virtual void OnRecoverFromFallBack() override;

	/**********************************************/
	/*               Aim Camera View              */
	/**********************************************/
private:
	virtual void OnAimStateChanged(bool bIsAiming) override;

	UPROPERTY(EditDefaultsOnly, Category = "View")
	FVector CameraAimLocalOffset;

	UPROPERTY(EditDefaultsOnly, Category = "View")
	float CameraLerpSpeed = 20.f;

	FTimerHandle CameraLerpTimerHandle;
	FVector CameraLerpGoal;

	void LerpCameraToLocalOffsetLocation(const FVector& GoalLocation);
	void TickCameraLocalOffsetLerp();

	/**********************************************/
	/*               Gameplay Ability             */
	/**********************************************/

	UPROPERTY()
	class UCHeroAttributeSet* HeroAttributeSet;

};
