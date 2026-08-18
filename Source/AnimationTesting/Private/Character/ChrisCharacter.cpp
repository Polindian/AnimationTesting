// Christopher Naglik All Rights Reserved


#include "Character/ChrisCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GAS/ChrisAbilitySystemComponent.h"
#include "GAS/ChrisAttributeSet.h"
#include "GAS/ChrisAbilitySystemStatics.h"
#include "Widgets/OverheadStatsGauge.h"
#include "GameFramework/CharacterMovementComponent.h" 
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Player/ChrisPlayerCharacter.h"
#include "Weapon/SwordEquipComponent.h"
#include "Widgets/ValueGauge.h"
#include "Components/WidgetComponent.h"
#include "Audio/ChrisAudioSubsystem.h"
#include "Audio/ChrisGameplayTags.h"


// Sets default values
AChrisCharacter::AChrisCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ChrisAbilitySystemComponent = CreateDefaultSubobject<UChrisAbilitySystemComponent>("Chris Ability System Component");
	ChrisAttributeSet = CreateDefaultSubobject<UChrisAttributeSet>("Chris Attribute Set");

	OverheadWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Overhead Widget Component"));
	OverheadWidgetComponent->SetupAttachment(GetRootComponent());

	// ======================================================
   // Screen space: widget renders as 2D overlay pinned to
   // the actor's position.
   // ======================================================
	OverheadWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	OverheadWidgetComponent->SetDrawAtDesiredSize(false);
	OverheadWidgetComponent->SetDrawSize(FVector2D(200.f, 60.f));

	BindGASChangeDelegate();

	PerceptionStimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>("Perception Stimuli Source Component");
}

void AChrisCharacter::ServerSideInit()
{
	ChrisAbilitySystemComponent->InitAbilityActorInfo(this, this);
	ChrisAbilitySystemComponent->ServerSideInit();
}

void AChrisCharacter::ClientSideInit()
{
	ChrisAbilitySystemComponent->InitAbilityActorInfo(this, this);
	ChrisAbilitySystemComponent->BindSkillCooldownAudio();
}

bool AChrisCharacter::IsLocallyControlledByPlayer() const
{
    return GetController() && GetController()->IsLocalPlayerController();
}

void AChrisCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
	if (NewController && !NewController->IsPlayerController())
	{
		ServerSideInit();
	}
	
}

void AChrisCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AChrisCharacter, TeamID);
}

// Called when the game starts or when spawned
void AChrisCharacter::BeginPlay()
{
	Super::BeginPlay();
	ConfigureOverheadStatusWidget();
	MeshRelativeTransform = GetMesh()->GetRelativeTransform();

	PerceptionStimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());
}

const TMap<EChrisAbilityInputID, TSubclassOf<UGameplayAbility>>& AChrisCharacter::GetAbilities() const
{
	return ChrisAbilitySystemComponent->GetAbilities();
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

bool AChrisCharacter::Server_SendGameplayEventToSelf_Validate(const FGameplayTag& EventTag, const FGameplayEventData& EventData)
{
	return true;
}

void AChrisCharacter::Server_SendGameplayEventToSelf_Implementation(const FGameplayTag& EventTag, const FGameplayEventData& EventData)
{
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventTag, EventData);
}




void AChrisCharacter::UpgradeAbilityWithInputID(EChrisAbilityInputID InputID)
{
	if(ChrisAbilitySystemComponent)
	{
		ChrisAbilitySystemComponent->Server_UpgradeAbilityWithID(InputID);
	}
}

void AChrisCharacter::BindGASChangeDelegate()
{
	if (ChrisAbilitySystemComponent)
	{
		ChrisAbilitySystemComponent->RegisterGameplayTagEvent(UChrisAbilitySystemStatics::GetDeadStatsTag()).AddUObject(this, &AChrisCharacter::DeathTagUpdated);
		ChrisAbilitySystemComponent->RegisterGameplayTagEvent(UChrisAbilitySystemStatics::GetStunStatsTag()).AddUObject(this, &AChrisCharacter::StunTagUpdated);
		ChrisAbilitySystemComponent->RegisterGameplayTagEvent(UChrisAbilitySystemStatics::GetFallBackStatsTag()).AddUObject(this, &AChrisCharacter::FallBackTagUpdated);
		ChrisAbilitySystemComponent->RegisterGameplayTagEvent(UChrisAbilitySystemStatics::GetAimStatsTag()).AddUObject(this, &AChrisCharacter::AimTagUpdated);

		ChrisAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UChrisAttributeSet::GetMoveSpeedAttribute()).AddUObject(this, &AChrisCharacter::MoveSpeedUpdated);
		ChrisAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UChrisAttributeSet::GetMoveAccelerationAttribute()).AddUObject(this, &AChrisCharacter::MoveAccelerationUpdated);

		ChrisAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UChrisAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &AChrisCharacter::MaxHealthUpdated);
		ChrisAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UChrisAttributeSet::GetMaxManaAttribute()).AddUObject(this, &AChrisCharacter::MaxManaUpdated);
		
	}
}

void AChrisCharacter::DeathTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount != 0)
	{
		if (IsLocallyControlledByPlayer())
		{
			if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
			{
				Audio->Play2D(ChrisGameplayTags::Audio_Player_Death);
			}
		}

		StartDeathSequence();

		if (ChrisAbilitySystemComponent)
		{
			ChrisAbilitySystemComponent->StopLowHealthLoop();
		}
	}
	else
	{
		Respawn();
	}
}

void AChrisCharacter::ConfigureOverheadStatusWidget()
{
    if (!OverheadWidgetComponent)
    {
        return;
    }

    if(IsLocallyControlledByPlayer())
    {
        OverheadWidgetComponent->SetHiddenInGame(true);
        return;
	}

	UOverheadStatsGauge* OverheadStatsGauge = Cast<UOverheadStatsGauge>(OverheadWidgetComponent->GetUserWidgetObject());
    if (OverheadStatsGauge)
    {
        OverheadStatsGauge->ConfigureWithASC(ChrisAbilitySystemComponent);
		OverheadStatsGauge->SetTextVisibility(false);
		OverheadStatsGauge->ConfigureTeamColor(this);
		OverheadWidgetComponent->SetHiddenInGame(false);
		GetWorldTimerManager().ClearTimer(HeadStatGaugeVisibilityUpdateTimerHandle);
		GetWorldTimerManager().SetTimer(HeadStatGaugeVisibilityUpdateTimerHandle, this, &AChrisCharacter::UpdateHeadStatGaugeVisibility, HeadStatGaugeVisibilityCheckUpdateGap, true);
	}
}

void AChrisCharacter::UpdateHeadStatGaugeVisibility()
{
	APawn* LocalPlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!LocalPlayerPawn || !OverheadWidgetComponent) return;

	float Distance = FVector::Dist(GetActorLocation(), LocalPlayerPawn->GetActorLocation());

	// ======================================================
	// Hide completely beyond max range
	// ======================================================
	if (Distance > OverheadWidgetMaxDistance)
	{
		OverheadWidgetComponent->SetHiddenInGame(true);
		return;
	}

	OverheadWidgetComponent->SetHiddenInGame(false);

	// ======================================================
	// Scale the widget like perspective: full size at the
	// reference distance, smaller as you move further away.
	// ======================================================
	float Scale = FMath::Clamp(
		OverheadWidgetReferenceDistance / FMath::Max(Distance, 1.f),
		0.4f,   // minimum scale at far distance
		1.0f    // maximum scale (at or closer than reference distance)
	);

	OverheadWidgetComponent->SetDrawSize(OverheadWidgetBaseSize * Scale);

	// Squish the widget vertically at distance — this compresses the gap between bars
	float HeightSquish = FMath::GetMappedRangeValueClamped(
		FVector2D(0.4f, 1.0f),   // Scale range
		FVector2D(0.5f, 1.0f),   // Squish range: 0.5 = half height at max distance
		Scale
	);

	UUserWidget* Widget = OverheadWidgetComponent->GetUserWidgetObject();
	if (Widget)
	{
		Widget->SetRenderTransform(FWidgetTransform(FVector2D::ZeroVector, FVector2D(1.f, HeightSquish), FVector2D::ZeroVector, 0.f));

		UOverheadStatsGauge* Gauge = Cast<UOverheadStatsGauge>(Widget);
		if (Gauge)
		{
			if (UChrisAbilitySystemStatics::IsHero(this))
			{
				Gauge->SetTextVisibility(Distance <= 800.f);
			}
			else
			{
				Gauge->SetTextVisibility(false);
			}
		}
	}
}

void AChrisCharacter::SetStatusGaugeEnabled(bool bIsEnabled)
{
	GetWorldTimerManager().ClearTimer(HeadStatGaugeVisibilityUpdateTimerHandle);

	if (bIsEnabled)
	{
		ConfigureOverheadStatusWidget();
	}
	else
	{
		OverheadWidgetComponent->SetHiddenInGame(true);
	}
}

void AChrisCharacter::DeathMontageFinished()
{
	UCapsuleComponent* Capsule = GetCapsuleComponent();

	if (Capsule)
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	}

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(RespawnDelayTimerHandle, this, &AChrisCharacter::RespawnImmediately, RespawnDelay);
	}
}

void AChrisCharacter::SetRagdollEnabled(bool bIsEnabled)
{
	if (bIsEnabled)
	{
		GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	}
	else
	{
		GetMesh()->SetSimulatePhysics(false);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetMesh()->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		GetMesh()->SetRelativeTransform(MeshRelativeTransform);
	}
}

void AChrisCharacter::PlayDeathAnimation()
{
	if (DeathMontage)
	{
		float MontageDuration = PlayAnimMontage(DeathMontage);
		GetWorldTimerManager().SetTimer(DeathMontageTimerHandle, this, &AChrisCharacter::DeathMontageFinished, MontageDuration + DeathMontageFinishTimeShift);
	}
}

void AChrisCharacter::StartDeathSequence()
{
	OnDead();

	if (ChrisAbilitySystemComponent)
	{
		ChrisAbilitySystemComponent->CancelAllAbilities();
	}

	PlayDeathAnimation();
	UE_LOG(LogTemp, Warning, TEXT("Dead"))
	SetStatusGaugeEnabled(false);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	SetAIPerceptionStimuliSourceEnabled(false);

}

void AChrisCharacter::StunTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if (IsDead())
	{
		return;
	}

	if (NewCount != 0)
	{
		OnStun();
		PlayAnimMontage(StunMontage);
	}
	else
	{
		OnRecoverFromStun();
		StopAnimMontage(StunMontage);
	}
}

void AChrisCharacter::FallBackTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if (IsDead())
	{
		return;
	}

	if (NewCount != 0)
	{
		OnFallBack();
		PlayAnimMontage(FallBackMontage);
	}
	else
	{
		OnRecoverFromFallBack();
		StopAnimMontage(FallBackMontage);
	}
}

void AChrisCharacter::AimTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	SetIsAiming(NewCount != 0);
}

void AChrisCharacter::SetIsAiming(bool bIsAiming)
{
	bUseControllerRotationYaw = bIsAiming;
	GetCharacterMovement()->bOrientRotationToMovement = !bIsAiming;
	GetCharacterMovement()->bUseControllerDesiredRotation = bIsAiming;
	OnAimStateChanged(bIsAiming);
}

void AChrisCharacter::OnAimStateChanged(bool bIsAiming)
{
}

void AChrisCharacter::MoveSpeedUpdated(const FOnAttributeChangeData& Data)
{
	UE_LOG(LogTemp, Warning, TEXT("MoveSpeedUpdated: %f on %s"), Data.NewValue, *GetName());
	GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
}

void AChrisCharacter::MoveAccelerationUpdated(const FOnAttributeChangeData& Data)
{
	GetCharacterMovement()->MaxAcceleration = Data.NewValue;
}

void AChrisCharacter::MaxHealthUpdated(const FOnAttributeChangeData& Data)
{
	if(IsValid(ChrisAttributeSet))
	{
		ChrisAttributeSet->RescaleHealth();
	}
}

void AChrisCharacter::MaxManaUpdated(const FOnAttributeChangeData& Data)
{
	if(IsValid(ChrisAttributeSet))
	{
		ChrisAttributeSet->RescaleMana();
	}
}

void AChrisCharacter::OnStun()
{

}

void AChrisCharacter::OnRecoverFromStun()
{

}

void AChrisCharacter::OnFallBack()
{
}

void AChrisCharacter::OnRecoverFromFallBack()
{
}



bool AChrisCharacter::IsDead() const
{
	return GetAbilitySystemComponent()->HasMatchingGameplayTag(UChrisAbilitySystemStatics::GetDeadStatsTag());
}

void AChrisCharacter::RespawnImmediately()
{
	if (HasAuthority())
	{
		GetAbilitySystemComponent()->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(UChrisAbilitySystemStatics::GetDeadStatsTag()));
	}
}

void AChrisCharacter::ForceResetFromDeath()
{
	CancelDeathTimers();

	// Only reset if actually dead
	if (!IsDead()) return;

	// Reset physical state (same as Respawn but without input/teleport/perception)
	SetRagdollEnabled(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	GetMesh()->GetAnimInstance()->StopAllMontages(0.f);
	SetStatusGaugeEnabled(true);

	if (USwordEquipComponent* SwordComp = FindComponentByClass<USwordEquipComponent>())
	{
		SwordComp->ResetToUnequipped();
	}

	bSuppressRespawnInput = true;

	// Remove the death tag (this will trigger DeathTagUpdated → Respawn)
	// So we need to guard Respawn — see below
	if (HasAuthority())
	{
		GetAbilitySystemComponent()->RemoveActiveEffectsWithGrantedTags(
			FGameplayTagContainer(UChrisAbilitySystemStatics::GetDeadStatsTag()));
	}

	// Apply full stats after removing death
	if (ChrisAbilitySystemComponent)
	{
		ChrisAbilitySystemComponent->ApplyFullStatEffect();
	}
}

void AChrisCharacter::CancelDeathTimers()
{
	GetWorldTimerManager().ClearTimer(DeathMontageTimerHandle);
	GetWorldTimerManager().ClearTimer(RespawnDelayTimerHandle);
}

void AChrisCharacter::Respawn()
{
	if (bSuppressRespawnInput)
	{
		// ForceResetFromDeath already handled everything
		bSuppressRespawnInput = false;
		return;
	}

	if (ChrisAbilitySystemComponent)
	{
		ChrisAbilitySystemComponent->StopLowHealthLoop();
	}
	OnRespawn();
	SetAIPerceptionStimuliSourceEnabled(true);
	SetRagdollEnabled(false);
	UE_LOG(LogTemp, Warning, TEXT("Respawned"))
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	GetMesh()->GetAnimInstance()->StopAllMontages(0.f);
	SetStatusGaugeEnabled(true);

	// Reset sword equip state so player respawns unequipped
	if (USwordEquipComponent* SwordComp = FindComponentByClass<USwordEquipComponent>())
	{
		SwordComp->ResetToUnequipped();
	}


	if (HasAuthority() && GetController())
	{
		TWeakObjectPtr<AActor> StartSpot = GetController()->StartSpot;
		if (StartSpot.IsValid())
		{
			SetActorTransform(StartSpot->GetActorTransform());
		}
	}

	if (ChrisAbilitySystemComponent)
	{
		ChrisAbilitySystemComponent->ApplyFullStatEffect();
	}
}

void AChrisCharacter::OnDead()
{
}

void AChrisCharacter::OnRespawn()
{
}

void AChrisCharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID = NewTeamID;
}

FGenericTeamId AChrisCharacter::GetGenericTeamId() const
{
	return TeamID;
}

void AChrisCharacter::OnRep_TeamID()
{
	UOverheadStatsGauge* OverheadStatsGauge = Cast<UOverheadStatsGauge>(
		OverheadWidgetComponent->GetUserWidgetObject());
	if (OverheadStatsGauge)
	{
		OverheadStatsGauge->ConfigureTeamColor(this);
	}
}

void AChrisCharacter::SetAIPerceptionStimuliSourceEnabled(bool bIsEnabled)
{
	if (!PerceptionStimuliSourceComponent)
	{
		return;
	}

	if (bIsEnabled)
	{
		PerceptionStimuliSourceComponent->RegisterWithPerceptionSystem();
	}
	else
	{
		PerceptionStimuliSourceComponent->UnregisterFromPerceptionSystem();
	}
}

