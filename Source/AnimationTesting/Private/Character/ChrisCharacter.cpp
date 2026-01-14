// Christopher Naglik All Rights Reserved


#include "Character/ChrisCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "GAS/ChrisAbilitySystemComponent.h"
#include "GAS/ChrisAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h" 
#include "Net/UnrealNetwork.h"    


// Sets default values
AChrisCharacter::AChrisCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ChrisAbilitySystemComponent = CreateDefaultSubobject<UChrisAbilitySystemComponent>("Chris Ability System Component");
	ChrisAttributeSet = CreateDefaultSubobject<UChrisAttributeSet>("Chris Attribute Set");

	// ADD: Configure crouch settings
	// WHY: UE5's built-in crouch handles capsule resize and replication automatically
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->CrouchedHalfHeight = 60.0f;      // Adjust to your ALS setup
	GetCharacterMovement()->MaxWalkSpeedCrouched = 400.0f;   // Your desired crouch speed
}

// ADD: This function tells UE5 which variables to replicate
// WHY: Without this, CurrentGait won't sync between server and clients
void AChrisCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // DOREPLIFETIME = "Do Replicate Lifetime"
    // This registers CurrentGait for replication
    DOREPLIFETIME(AChrisCharacter, CurrentGait);
}
// ADD: Public function called by input (Blueprint or C++)
// WHY: This is the entry point - checks if we need to tell server
void AChrisCharacter::RequestCrouch()
{
    if (GetLocalRole() < ROLE_Authority)
    {
        // We are a client, ask server to crouch us
        Server_Crouch();
    }
    else
    {
        // We are the server, crouch directly
        Server_Crouch_Implementation();
    }
}
void AChrisCharacter::RequestUnCrouch()
{
    if (GetLocalRole() < ROLE_Authority)
    {
        Server_UnCrouch();
    }
    else
    {
        Server_UnCrouch_Implementation();
    }
}

// ADD: Server RPC implementation
// WHY: "_Implementation" suffix is required by UE5 for Server/Client RPCs
void AChrisCharacter::Server_Crouch_Implementation()
{
    // Use UE5's built-in Crouch() - handles capsule and replication
    Crouch();

    // Update our custom gait (will replicate via OnRep_CurrentGait)
    CurrentGait = EChrisGait::Crouching;
}
void AChrisCharacter::Server_UnCrouch_Implementation()
{
    UnCrouch();
    CurrentGait = EChrisGait::Running;  // Or Walking, depending on your logic
}
// ADD: Called automatically when CurrentGait replicates to a client
// WHY: Lets the client update local visuals/animations when server changes gait
void AChrisCharacter::OnRep_CurrentGait()
{
    // This fires on CLIENTS when the server changes CurrentGait
    // Your Animation Blueprint can read GetCurrentGait() and it will be correct

    // Optional: Force animation update or trigger events here
}

void AChrisCharacter::ServerSideInit()
{
	ChrisAbilitySystemComponent->InitAbilityActorInfo(this, this);
	ChrisAbilitySystemComponent->ApplyInitialEffects();
}

void AChrisCharacter::ClientSideInit()
{
	ChrisAbilitySystemComponent->InitAbilityActorInfo(this, this);
}

// Called when the game starts or when spawned
void AChrisCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AChrisCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AChrisCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

UAbilitySystemComponent* AChrisCharacter::GetAbilitySystemComponent() const
{
	return ChrisAbilitySystemComponent;
}

