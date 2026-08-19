// Christopher Naglik All Rights Reserved

#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_ThrustTrail.generated.h"

UCLASS(meta = (DisplayName = "Thrust Trail"))
class UAnimNotifyState_ThrustTrail : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;

    UPROPERTY(EditAnywhere, Category = "Thrust Trail")
    TObjectPtr<class UNiagaraSystem> TrailSystem;

    // Must match the user parameter name on the Niagara system exactly
    UPROPERTY(EditAnywhere, Category = "Thrust Trail")
    FString SkeletalMeshParameterName = TEXT("Skeletal Mesh");
};