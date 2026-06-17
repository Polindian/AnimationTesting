// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Animations/AN_SendGameplayEvent.h"
#include "AN_RollSpeedBlend.generated.h"

/**
 * 
 */
UCLASS()
class UAN_RollSpeedBlend : public UAN_SendGameplayEvent
{
    GENERATED_BODY()
public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;

private:
    UPROPERTY(EditAnywhere, Category = "Movement")
    float BlendToSpeed = 400.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float BlendDuration = 0.6f;
};

