// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RoundTimerWidget.generated.h"

// Displays the round timer at the top of the screen
UCLASS()
class URoundTimerWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Called by PlayerController to start the timer display.
    void StartTimer(float Duration);

protected:
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    // The world time (in seconds since game start) when the timer should hit 0.
    float EndTime = 0.f;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TimerText;
};
