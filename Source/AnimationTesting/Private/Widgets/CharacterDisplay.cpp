// Christopher Naglik All Rights Reserved


#include "Widgets/CharacterDisplay.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/PA_CharacterDefinition.h"
#include "Components/StaticMeshComponent.h"
#include "GroomComponent.h"
#include "Weapon/DisplaySwordEquipComponent.h"

// Sets default values
ACharacterDisplay::ACharacterDisplay()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("Root Comp"));

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh Comp");
	MeshComponent->SetupAttachment(GetRootComponent());


	ViewCameraComponent = CreateDefaultSubobject<UCameraComponent>("View Camera Comp");
	ViewCameraComponent->SetupAttachment(GetRootComponent());
}



void ACharacterDisplay::ConfigureWithCharacterDefinition(const UPA_CharacterDefinition* CharacterDefinition)
{
	if (!CharacterDefinition)
		return;

    UE_LOG(LogTemp, Warning, TEXT("Mesh: %s | AnimBP: %s"),
        *GetNameSafe(CharacterDefinition->LoadDisplayMesh()),
        *GetNameSafe(CharacterDefinition->LoadDisplayAnimationBP()));

    MeshComponent->SetSkeletalMesh(CharacterDefinition->LoadDisplayMesh());
    MeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
    MeshComponent->SetAnimClass(CharacterDefinition->LoadDisplayAnimationBP());
    

    ClearDisplayComponents();

    
   
    // Spawn costume pieces — each follows the main body's animation via SetLeaderPoseComponent
    for (const FDisplayCostumePiece& Piece : CharacterDefinition->GetDisplayCostumePieces())
    {
        USkeletalMesh* PieceMesh = Piece.Mesh.LoadSynchronous();
        if (!PieceMesh) continue;

        USkeletalMeshComponent* NewComp = NewObject<USkeletalMeshComponent>(this);
        NewComp->SetupAttachment(MeshComponent);
        NewComp->SetSkeletalMesh(PieceMesh);
        NewComp->SetLeaderPoseComponent(MeshComponent);
        NewComp->RegisterComponent();
        CostumeMeshComponents.Add(NewComp);
    }

    // Spawn weapons — static meshes attached to named sockets on the main skeleton
    for (const FDisplayWeaponPiece& Weapon : CharacterDefinition->GetDisplayWeapons())
    {
        UStaticMesh* WeaponMesh = Weapon.Mesh.LoadSynchronous();
        if (!WeaponMesh) continue;

        UStaticMeshComponent* NewComp = NewObject<UStaticMeshComponent>(this);
        // Attach to the specified socket (e.g., "sword_left", "sword_right")
        NewComp->SetupAttachment(MeshComponent, Weapon.AttachSocket);
        NewComp->SetStaticMesh(WeaponMesh);
        NewComp->RegisterComponent();
        WeaponMeshComponents.Add(NewComp);
    }

    // Spawn groom assets (hair, eyebrows, etc.) — attaches to the correct skeletal mesh
    for (const FDisplayGroomPiece& Groom : CharacterDefinition->GetDisplayGrooms())
    {
        UGroomAsset* LoadedGroom = Groom.GroomAsset.LoadSynchronous();
        if (!LoadedGroom) continue;

        UGroomComponent* NewGroom = NewObject<UGroomComponent>(this);

        // Determine which mesh this groom attaches to
        USkeletalMeshComponent* TargetMesh = MeshComponent; // default: main body
        if (Groom.TargetMeshIndex >= 0 && Groom.TargetMeshIndex < CostumeMeshComponents.Num())
        {
            TargetMesh = CostumeMeshComponents[Groom.TargetMeshIndex];
        }

        NewGroom->SetupAttachment(TargetMesh);
        NewGroom->SetGroomAsset(LoadedGroom);

        // Binding asset maps groom curves to the target mesh's vertices
        UGroomBindingAsset* LoadedBinding = Groom.BindingAsset.LoadSynchronous();
        if (LoadedBinding)
        {
            NewGroom->SetBindingAsset(LoadedBinding);
        }

        NewGroom->RegisterComponent();
        GroomComponents.Add(NewGroom);
    }

    if (WeaponMeshComponents.Num() >= 2)
    {
        if (!DisplaySwordEquip)
        {
            DisplaySwordEquip = NewObject<UDisplaySwordEquipComponent>(this);
            DisplaySwordEquip->RegisterComponent();
        }

        // Index 0 = left sword, Index 1 = right sword
        DisplaySwordEquip->Initialize(MeshComponent, WeaponMeshComponents[0], WeaponMeshComponents[1]);
    }
}



void ACharacterDisplay::ClearDisplayComponents()
{
    
    // Destroy old costume pieces so switching characters doesn't stack meshes
    for (USkeletalMeshComponent* Comp : CostumeMeshComponents)
    {
        if (Comp)
        {
            Comp->DestroyComponent();
        }
    }
    CostumeMeshComponents.Empty();

    // Destroy old weapon meshes
    for (UStaticMeshComponent* Comp : WeaponMeshComponents)
    {
        if (Comp)
        {
            Comp->DestroyComponent();
        }
    }
    WeaponMeshComponents.Empty();

    // Destroy old groom components
    for (UGroomComponent* Comp : GroomComponents)
    {
        if (Comp)
        {
            Comp->DestroyComponent();
        }
    }
    GroomComponents.Empty();
}

