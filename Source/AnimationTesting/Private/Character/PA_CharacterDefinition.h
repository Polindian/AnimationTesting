// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PA_CharacterDefinition.generated.h"

class UGroomAsset;
class UGroomBindingAsset;
class AChrisCharacter;

// A costume skeletal mesh that follows the main body's animation via SetLeaderPoseComponent
USTRUCT(BlueprintType)
struct FDisplayCostumePiece
{
	GENERATED_BODY()

	// The skeletal mesh asset for this costume piece (hair, armor, clothing, etc.)
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<USkeletalMesh> Mesh;
};

// A static mesh weapon attached to a specific socket on the main body
USTRUCT(BlueprintType)
struct FDisplayWeaponPiece
{
	GENERATED_BODY()

	// The static mesh asset for this weapon
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UStaticMesh> Mesh;

	// Socket on the main body skeleton to attach to (e.g., "sword_left", "sword_right")
	UPROPERTY(EditDefaultsOnly)
	FName AttachSocket;
};

USTRUCT(BlueprintType)
struct FDisplayGroomPiece
{
	GENERATED_BODY()

	// The groom asset (hair, eyebrows, beard, etc.)
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<class UGroomAsset> GroomAsset;

	// The groom binding asset — maps the groom curves to the target mesh's vertices
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<class UGroomBindingAsset> BindingAsset;

	// Which skeletal mesh this groom attaches to:
	// -1 = main body mesh, 0+ = index into DisplayCostumePieces array
	UPROPERTY(EditDefaultsOnly)
	int32 TargetMeshIndex = 4;
};


/**
 * 
 */
UCLASS()
class UPA_CharacterDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	static FPrimaryAssetType GetCharacterDefinitionAssetType();

	FString GetCharacterDisplayName() const { return CharacterName; }
	UTexture2D* LoadIcon() const;
	TSubclassOf<AChrisCharacter> LoadCharacterClass() const;
	TSubclassOf<UAnimInstance> LoadDisplayAnimationBP() const;
	class USkeletalMesh* LoadDisplayMesh() const;


	// Getters for display pieces
	const TArray<FDisplayCostumePiece>& GetDisplayCostumePieces() const { return DisplayCostumePieces; }
	const TArray<FDisplayWeaponPiece>& GetDisplayWeapons() const { return DisplayWeapons; }
	const TArray<FDisplayGroomPiece>& GetDisplayGrooms() const { return DisplayGrooms; }

private:
	UPROPERTY(EditDefaultsOnly, Category = "Character")
	FString CharacterName;

	UPROPERTY(EditDefaultsOnly, Category = "Character")
	TSoftObjectPtr<UTexture2D> CharacterIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Character")
	TSoftClassPtr<AChrisCharacter> CharacterClass;

	UPROPERTY(EditDefaultsOnly, Category = "Character")
	TSoftClassPtr<UAnimInstance> DisplayAnimBP;


	// Extra skeletal meshes that copy the main body's animation 
	UPROPERTY(EditDefaultsOnly, Category = "Display")
	TArray<FDisplayCostumePiece> DisplayCostumePieces;

	// Static mesh weapons that snap to named sockets on the main skeleton
	UPROPERTY(EditDefaultsOnly, Category = "Display")
	TArray<FDisplayWeaponPiece> DisplayWeapons;

	UPROPERTY(EditDefaultsOnly, Category = "Display")
	TArray<FDisplayGroomPiece> DisplayGrooms;
};
