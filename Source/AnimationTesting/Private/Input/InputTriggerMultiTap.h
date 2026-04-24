// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "InputTriggers.h"
#include "InputTriggerMultiTap.generated.h"

/**
 * 
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Multi Tap"))
class UInputTriggerMultiTap : public UInputTrigger
{
	GENERATED_BODY()
	

public:
    // Number of taps required to trigger
    UPROPERTY(EditAnywhere, Category = "Trigger Settings", meta = (ClampMin = "2"))
    int32 RequiredTapCount = 3;

    // Max time allowed between consecutive taps (seconds)
    UPROPERTY(EditAnywhere, Category = "Trigger Settings", meta = (ClampMin = "0.1"))
    float MaxTimeBetweenTaps = 0.4f;

    // Max hold duration to count as a "tap" vs a "hold"
    UPROPERTY(EditAnywhere, Category = "Trigger Settings", meta = (ClampMin = "0.05"))
    float MaxTapHoldDuration = 0.2f;

protected:
    virtual ETriggerState UpdateState_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue ModifiedValue, float DeltaTime) override;

private:
    int32 CurrentTapCount = 0;
    double LastTapTime = 0.0;
    bool bWasPressedLastFrame = false;
};
