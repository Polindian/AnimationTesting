// Christopher Naglik All Rights Reserved


#include "GAS/ChrisGameplayAbility.h"
#include "Components/SkeletalMeshComponent.h"
#include "GAS/ChrisAbilitySystemStatics.h"
#include "AbilitySystemComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystemBlueprintLibrary.h" 
#include "GameFramework/Character.h"
#include "GAS/GAP_Launched.h"

UChrisGameplayAbility::UChrisGameplayAbility()
{
    ActivationBlockedTags.AddTag(UChrisAbilitySystemStatics::GetStunStatsTag());
    ActivationBlockedTags.AddTag(UChrisAbilitySystemStatics::GetFallBackStatsTag());
}

bool UChrisGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	FGameplayAbilitySpec* AbilitySpec = ActorInfo->AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
    if(AbilitySpec && AbilitySpec->Level <= 0)
    {
        return false;
	}

	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

UAnimInstance* UChrisGameplayAbility::GetOwnerAnimInstance() const
{
    USkeletalMeshComponent* OwnerSkeletalMeshComp = GetOwningComponentFromActorInfo();
    if(OwnerSkeletalMeshComp)
    {
        return OwnerSkeletalMeshComp->GetAnimInstance();
	}
    return nullptr;
}

TArray<FHitResult> UChrisGameplayAbility::GetHitResultFromSweepLocationTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle, float SphereSweepRadius, ETeamAttitude::Type TargetTeam, bool bDrawDebug, bool bIgnoreSelf) const
{
    TArray<FHitResult> OutResult;
    TSet<AActor*> HitActors;

	IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(GetAvatarActorFromActorInfo());

    for (const TSharedPtr<FGameplayAbilityTargetData> TargetData : TargetDataHandle.Data)
    {
        FVector StartLocation = TargetData->GetOrigin().GetTranslation();
        FVector EndLocation = TargetData->GetEndPoint();

        TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
        ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

        TArray<AActor*> ActorsToIgnore;
        if (bIgnoreSelf)
        {
            ActorsToIgnore.Add(GetAvatarActorFromActorInfo());
        }

        EDrawDebugTrace::Type DrawDebugTrace = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

        TArray<FHitResult> Results;

        UKismetSystemLibrary::SphereTraceMultiForObjects(this, StartLocation, EndLocation, SphereSweepRadius, ObjectTypes, false, ActorsToIgnore, DrawDebugTrace, Results, false);

        for (const FHitResult& Result : Results)
        {
            if (HitActors.Contains(Result.GetActor()))
            {
                continue;
            }

            if (OwnerTeamInterface)
            {
				ETeamAttitude::Type OtherActorTeamAttitude = OwnerTeamInterface->GetTeamAttitudeTowards(*Result.GetActor());
                if (OtherActorTeamAttitude != TargetTeam)
                {
                    continue;
				}
                else
                {

                }
            }

            HitActors.Add(Result.GetActor());
            OutResult.Add(Result);
        }
    }
    return OutResult;
}

void UChrisGameplayAbility::PushSelf(const FVector& PushVelocity)
{
    ACharacter* OwningAvatarCharacter = GetOwningAvatarCharacter();
    if (OwningAvatarCharacter)
    {
        OwningAvatarCharacter->LaunchCharacter(PushVelocity, true, true);
    }
}

void UChrisGameplayAbility::PushTarget(AActor* Target, const FVector& PushVelocity)
{
    if (!Target) return;

    FGameplayEventData EventData;
    FGameplayAbilityTargetData_SingleTargetHit* HitData = new FGameplayAbilityTargetData_SingleTargetHit;
    FHitResult HitResult;
    HitResult.ImpactNormal = PushVelocity;
    HitData->HitResult = HitResult;
    EventData.TargetData.Add(HitData);

    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Target, UGAP_Launched::GetLaunchedAbilityActivationTag(), EventData);
}

ACharacter* UChrisGameplayAbility::GetOwningAvatarCharacter()
{
    if (!AvatarCharacter)
    {
        AvatarCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    }

    return AvatarCharacter;
}

void UChrisGameplayAbility::PushTargetsFromOwnerLocation(const FGameplayAbilityTargetDataHandle& TargetDataHandle, float PushSpeed)
{
    TArray<AActor*> TargetActors = UAbilitySystemBlueprintLibrary::GetAllActorsFromTargetData(TargetDataHandle);
    PushTargetsFromOwnerLocation(TargetActors, PushSpeed);
}

void UChrisGameplayAbility::PushTargetsFromOwnerLocation(const TArray<AActor*>& Targets, float PushSpeed)
{
    AActor* OwnerAvatarActor = GetAvatarActorFromActorInfo();
    if (!OwnerAvatarActor) return;

    FVector OwnerLocation = OwnerAvatarActor->GetActorLocation();
    PushTargetsFromLocation(Targets, OwnerLocation, PushSpeed);
}

void UChrisGameplayAbility::PushTargetsFromLocation(const TArray<AActor*>& Targets, const FVector& Location, float PushSpeed)
{
    for (AActor* Target : Targets)
    {
        if (!Target) continue;
        FVector PushDirection = (Target->GetActorLocation() - Location).GetSafeNormal();
        PushTarget(Target, PushDirection * PushSpeed);
    }
}

FGenericTeamId UChrisGameplayAbility::GetOwnerTeamId() const
{
	IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(GetAvatarActorFromActorInfo());
    if(OwnerTeamInterface)
    {
        return OwnerTeamInterface->GetGenericTeamId();
	}

	return FGenericTeamId::NoTeam;
}

void UChrisGameplayAbility::ApplyGameplayEffectToHitResultActor(const FHitResult& HitResult, TSubclassOf<UGameplayEffect> GameplayEffect, int Level)
{
    FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(GameplayEffect, Level);
    FGameplayEffectContextHandle EffectContext = MakeEffectContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
    EffectContext.AddHitResult(HitResult);
    EffectSpecHandle.Data->SetContext(EffectContext);

    ApplyGameplayEffectSpecToTarget(
        GetCurrentAbilitySpecHandle(),
        CurrentActorInfo,      
        CurrentActivationInfo, 
        EffectSpecHandle,
        UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitResult.GetActor())
    );
}



