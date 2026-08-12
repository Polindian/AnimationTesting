// Christopher Naglik All Rights Reserved

#include "GAS/BlindBombExecution.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GenericTeamAgentInterface.h"

void UBlindBombExecution::Execute_Implementation(
    const FGameplayEffectCustomExecutionParameters& ExecutionParams,
    FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
    // ======================================================
    // GET THE CASTER (source) — this is the player who used
    // the consumable. The execution runs on them.
    // ======================================================
    UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
    if (!SourceASC) return;

    AActor* SourceActor = SourceASC->GetAvatarActor();
    if (!SourceActor) return;

    // ======================================================
    // SPHERE OVERLAP: find all pawns within 240 units.
    // Same logic that was in GA_BlindBomb but now runs
    // inside the consumed GE's execution path.
    // ======================================================
    float BlindRadius = 240.f;

    TArray<FOverlapResult> Overlaps;
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(BlindRadius);
    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

    SourceActor->GetWorld()->OverlapMultiByObjectType(
        Overlaps,
        SourceActor->GetActorLocation(),
        FQuat::Identity,
        ObjectParams,
        SphereShape);

    // ======================================================
    // TEAM CHECK: only blind hostile actors.
    // ======================================================
    IGenericTeamAgentInterface* SourceTeam = Cast<IGenericTeamAgentInterface>(SourceActor);

    // ======================================================
    // LOAD THE BLIND GE: we reference GE_Blinded by path.
    // This is a soft reference so the execution calc doesn't
    // hard-depend on a specific Blueprint asset.
    // UPDATE THIS PATH to match where your GE_Blinded lives
    // in your content folder.
    // ======================================================
    UClass* BlindEffectClass = LoadClass<UGameplayEffect>(
        nullptr,
        TEXT("/Game/Main/Christopher/Characters/Chris/GameplayAbility/NightflareBlind/GE_Nightflare_Blind.GE_Nightflare_Blind_C"));

    if (!BlindEffectClass)
    {
        UE_LOG(LogTemp, Error, TEXT("[BlindBomb] Could not load GE_Blinded! Check the path."));
        return;
    }

    // ======================================================
    // APPLY GE_BLINDED to each enemy in range.
    // We build the spec from the source ASC so the "source"
    // context is correctly set to the caster — this means
    // the Gameplay Cue fires on each TARGET's client.
    // ======================================================
    TSet<AActor*> BlindedActors;
    FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
    ContextHandle.AddSourceObject(SourceActor);

    FGameplayEffectSpecHandle BlindSpec = SourceASC->MakeOutgoingSpec(
        BlindEffectClass, 1, ContextHandle);

    if (!BlindSpec.IsValid()) return;

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* Target = Overlap.GetActor();

        // Skip self, null, already-processed
        if (!Target || Target == SourceActor || BlindedActors.Contains(Target))
        {
            continue;
        }

        // Only blind hostiles
        if (SourceTeam && SourceTeam->GetTeamAttitudeTowards(*Target) != ETeamAttitude::Hostile)
        {
            continue;
        }

        UAbilitySystemComponent* TargetASC =
            UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);

        if (TargetASC)
        {
            SourceASC->ApplyGameplayEffectSpecToTarget(
                *BlindSpec.Data.Get(), TargetASC);

            BlindedActors.Add(Target);
            UE_LOG(LogTemp, Warning, TEXT("[BlindBomb] Blinded: %s"), *Target->GetName());
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[BlindBomb] Blinded %d enemies within %.0f units"),
        BlindedActors.Num(), BlindRadius);
}