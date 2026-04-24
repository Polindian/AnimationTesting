// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Weapon/SwordEquipComponent.h"
#include "AN_SwordPhase.generated.h"

/**
 * 
 */
UCLASS()
class UAN_SwordPhase : public UAnimNotify
{
	GENERATED_BODY()
	

public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;

private:
    // Set this in the montage editor for each notify instance:
    // - DetachAndThrow: at the frame where hands throw up
    // - BeginSnap: at the frame where you want swords to fly to target
    // - AttachToTarget: at the end, as a safety fallback
    UPROPERTY(EditAnywhere, Category = "Sword Phase")
    ESwordPhaseAction Action = ESwordPhaseAction::DetachAndThrow;

    virtual FString GetNotifyName_Implementation() const override;
};
