// Christopher Naglik All Rights Reserved


#include "GAS/GA_Shoot.h"
#include "GAS/ChrisAbilitySystemStatics.h"
#include "GAS/ProjectileActor.h"
#include "GAS/ChrisAttributeSet.h"
#include "GameplayTagsManager.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"

UGA_Shoot::UGA_Shoot()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	ActivationBlockedTags.AddTag(UChrisAbilitySystemStatics::GetWeaponsEquippedTag());
	ActivationRequiredTags.AddTag(UChrisAbilitySystemStatics::GetWeaponsUnequippedTag());
	ActivationOwnedTags.AddTag(UChrisAbilitySystemStatics::GetAimStatsTag());
	ActivationOwnedTags.AddTag(UChrisAbilitySystemStatics::GetCrosshairTag());
}

void UGA_Shoot::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character || !IdleToAimMontage)
	{
		K2_EndAbility();
		return;
	}

	// Only reset shot count if cooldown is not active
	

	Character->PlayAnimMontage(IdleToAimMontage);

	UE_LOG(LogTemp, Warning, TEXT("Shoot Ability Activated!"));


	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		// Listen for RMB RELEASED — this is how we know the player stopped aiming.
		// When received, StopShooting plays AimToIdle and ends the ability.
		UAbilityTask_WaitGameplayEvent* WaitStopShootingEvent =UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, UChrisAbilitySystemStatics::GetHeavyAttack3InputReleasedTag());
		WaitStopShootingEvent->EventReceived.AddDynamic(this, &UGA_Shoot::StopShooting);
		WaitStopShootingEvent->ReadyForActivation();

		// Listen for shoot input
		UAbilityTask_WaitGameplayEvent* WaitShootProjectileEvent =UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetShootTag(), nullptr, false, false);
		WaitShootProjectileEvent->EventReceived.AddDynamic(this, &UGA_Shoot::ShootProjectile);
		WaitShootProjectileEvent->ReadyForActivation();

		UE_LOG(LogTemp, Warning, TEXT("[GA_Shoot] Tasks created - listening for shoot tag: %s"), *GetShootTag().ToString());
	}

	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GA_Shoot] ActivateAbility - NO authority or prediction key! Tasks NOT created."));
	}

}

void UGA_Shoot::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character && AimToIdleMontage)
	{
		Character->StopAnimMontage(nullptr);
		UE_LOG(LogTemp, Warning, TEXT("Stopped Aiming!"));
	}

	K2_EndAbility();
}

bool UGA_Shoot::CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	return true;
}

void UGA_Shoot::GetCooldownTimeRemainingAndDuration(FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, float& TimeRemaining, float& CooldownDuration) const
{
	CooldownDuration = ActiveCooldownDuration > 0.f ? ActiveCooldownDuration : ShootCooldownDuration;
	TimeRemaining = GetWorld() ? FMath::Max(0.f, (float)(CooldownEndTime - GetWorld()->GetTimeSeconds())) : 0.f;
}

FGameplayTag UGA_Shoot::GetShootTag()
{
	return UChrisAbilitySystemStatics::GetHeavyAttack3ShootInputTag();
}


void UGA_Shoot::StopShooting(FGameplayEventData Payload)
{

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character && AimToIdleMontage)
	{
		Character->StopAnimMontage(nullptr);
	}

	K2_EndAbility();
}

void UGA_Shoot::ShootProjectile(FGameplayEventData Payload)
{

	double CurrentTime = GetWorld()->GetTimeSeconds();
	if (IsCooldownActive() || !ShootMontage || (CurrentTime - LastShotTime) < 0.15) return;
	LastShotTime = CurrentTime;

	UAbilityTask_PlayMontageAndWait* PlayShootMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ShootMontage);
	PlayShootMontageTask->ReadyForActivation();
	
	if (K2_HasAuthority())
	{
		AActor* OwnerAvatarActor = GetAvatarActorFromActorInfo();
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerAvatarActor;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


		// Spawn at hand_r socket directly
		FVector SocketLocation = OwnerAvatarActor->GetActorLocation(); // fallback
		ACharacter* ShootingCharacter = Cast<ACharacter>(OwnerAvatarActor);
		if (ShootingCharacter && ShootingCharacter->GetMesh())
		{
			SocketLocation = ShootingCharacter->GetMesh()->GetSocketLocation(FName("ProjectileSpawn"));
			UE_LOG(LogTemp, Warning, TEXT("[GA_Shoot] Projectile spawn at: %s"), *SocketLocation.ToString());
		}

		AProjectileActor* Projectile = GetWorld()->SpawnActor<AProjectileActor>(ProjectileClass, SocketLocation, OwnerAvatarActor->GetActorRotation(), SpawnParams);
		if (Projectile)
		{
			Projectile->ShootProjectile(ShootProjectileSpeed, ShootProjectileRange, nullptr, GetOwnerTeamId(), 
				MakeOutgoingGameplayEffectSpec(ProjectileHitEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo)));
		}
	}

	CurrentShotCount++;

	UE_LOG(LogTemp, Warning, TEXT("[GA_Shoot] Shot fired! CurrentShotCount=%d / MaxShots=%d"), CurrentShotCount, MaxShots);

	if (CurrentShotCount >= MaxShots)
	{
		CurrentShotCount = 0;
		ActiveCooldownDuration = GetLeveledCooldownDuration();
		CooldownEndTime = GetWorld()->GetTimeSeconds() + ActiveCooldownDuration;

		CommitAbilityCost(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);

		GetWorld()->GetTimerManager().SetTimer(
			ShootCooldownTimerHandle, this, &UGA_Shoot::OnShootCooldownFinished, ActiveCooldownDuration);

		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			ASC->AbilityCommittedCallbacks.Broadcast(this);
		}
	}
}

float UGA_Shoot::GetLeveledCooldownDuration() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		UGameplayEffect* CooldownEffect = GetCooldownGameplayEffect();
		if (CooldownEffect)
		{
			int32 Level = GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo);
			FGameplayEffectSpecHandle EffectSpec = ASC->MakeOutgoingSpec(
				CooldownEffect->GetClass(), Level, ASC->MakeEffectContext());
			float CalcDuration = 0.f;
			CooldownEffect->DurationMagnitude.AttemptCalculateMagnitude(
				*EffectSpec.Data.Get(), CalcDuration);
			return FMath::Abs(CalcDuration);
		}
	}
	return ShootCooldownDuration;  // fallback if no GE set
}

bool UGA_Shoot::IsCooldownActive() const
{
	return GetWorld() && GetWorld()->GetTimeSeconds() < CooldownEndTime;
}

void UGA_Shoot::OnShootCooldownFinished()
{
	CurrentShotCount = 0;

	// ======================================================
	// DEADEYE RESET: clear the hit counter at the start of
	// each new volley. This prevents partial hits from the
	// previous volley carrying over.
	// ======================================================
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		bool bFound = false;
		float DeadeyeFlag = ASC->GetGameplayAttributeValue(
			UChrisAttributeSet::GetDeadeyeActiveAttribute(), bFound);

		if (bFound && DeadeyeFlag > 0.f)
		{
			ASC->SetNumericAttributeBase(
				UChrisAttributeSet::GetDeadeyeHitCounterAttribute(), 0.f);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[GA_Shoot] OnShootCooldownFinished!"));
}

