// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CharacterDisplay.generated.h"

class UPA_CharacterDefinition;

UCLASS()
class ACharacterDisplay : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACharacterDisplay();
	void ConfigureWithCharacterDefinition(const UPA_CharacterDefinition* CharacterDefinition);

private:
	UPROPERTY(VisibleDefaultsOnly, Category = "Character Display")
	class USkeletalMeshComponent* MeshComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = "Character Display")
	class UCameraComponent* ViewCameraComponent;

	// Dynamically spawned costume pieces 
	UPROPERTY()
	TArray<USkeletalMeshComponent*> CostumeMeshComponents;

	// Dynamically spawned weapon meshes 
	UPROPERTY()
	TArray<UStaticMeshComponent*> WeaponMeshComponents;

	UPROPERTY()
	TArray<class UGroomComponent*> GroomComponents;

	// Destroys all previously spawned costume and weapon components
	void ClearDisplayComponents();
};
