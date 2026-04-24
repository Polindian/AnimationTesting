// Christopher Naglik All Rights Reserved


#include "Input/InputTriggerMultiTap.h"

ETriggerState UInputTriggerMultiTap::UpdateState_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue ModifiedValue, float DeltaTime)
{
    const double CurrentTime = FPlatformTime::Seconds();
    const bool bIsPressed = ModifiedValue.Get<bool>();

    // Detect new press (key down this frame, wasn't down last frame)
    if (bIsPressed && !bWasPressedLastFrame)
    {
        // Reset if too much time since last tap
        if (CurrentTapCount > 0 && (CurrentTime - LastTapTime) > MaxTimeBetweenTaps)
        {
            CurrentTapCount = 0;
        }

        CurrentTapCount++;
        LastTapTime = CurrentTime;

        if (CurrentTapCount >= RequiredTapCount)
        {
            CurrentTapCount = 0;
            bWasPressedLastFrame = bIsPressed;
            return ETriggerState::Triggered;
        }
    }

    // Timeout reset
    if (CurrentTapCount > 0 && (CurrentTime - LastTapTime) > MaxTimeBetweenTaps)
    {
        CurrentTapCount = 0;
    }

    bWasPressedLastFrame = bIsPressed;
    return ETriggerState::None;
}
