// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Net/UnrealNetwork.h"    
#include "ChrisCharacter.generated.h"

UENUM(BlueprintType)
enum class EChrisGait : uint8
{
	Walking,
	Running,
	Crouching
};

UCLASS()
class AChrisCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AChrisCharacter();
	void ServerSideInit();
	void ClientSideInit();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	/********************************/
	 /*        Locomotion/ALS        */
	 /********************************/
public:
	// ADD: Called by client input, executes crouch on server
	UFUNCTION(BlueprintCallable, Category = "Locomotion")
	void RequestCrouch();
	UFUNCTION(BlueprintCallable, Category = "Locomotion")
	void RequestUnCrouch();
	// ADD: Getter for current gait (use in Animation Blueprint)
	UFUNCTION(BlueprintCallable, Category = "Locomotion")
	EChrisGait GetCurrentGait() const { return CurrentGait; }

protected:
	// ADD: Server RPC - client tells server to crouch
	// WHY: Only server can authoritatively change movement state
	UFUNCTION(Server, Reliable)
	void Server_Crouch();
	UFUNCTION(Server, Reliable)
	void Server_UnCrouch();
	// ADD: Called when CurrentGait replicates to clients
	// WHY: Lets clients react to gait changes (update animations, etc.)
	UFUNCTION()
	void OnRep_CurrentGait();
	// ADD: Replicated gait variable
	// WHY: Server sets this, automatically syncs to all clients
	UPROPERTY(ReplicatedUsing = OnRep_CurrentGait, BlueprintReadOnly, Category = "Locomotion")
	EChrisGait CurrentGait = EChrisGait::Running;


	/********************************/
	/*        GameplayAbility       */
	/********************************/

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
private:
	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay Ability")
	class UChrisAbilitySystemComponent* ChrisAbilitySystemComponent;
	UPROPERTY()
	class UChrisAttributeSet* ChrisAttributeSet;
};
