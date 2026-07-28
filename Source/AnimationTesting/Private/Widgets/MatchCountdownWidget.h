// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MatchCountdownWidget.generated.h"

// Displays the big countdown text in the center of the screen.
// The PlayerController calls UpdateCountdown() each second
UCLASS()
class UMatchCountdownWidget : public UUserWidget
{
    GENERATED_BODY()

public:

    // Called once when the countdown begins, so the colour ramp knows its range
    void StartCountdown(int32 TotalSeconds);

    // Called by PlayerController to update the displayed number.
    void UpdateCountdown(int32 SecondsRemaining);

private:
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* CountdownText;

    // Colour at the start of the countdown
    UPROPERTY(EditDefaultsOnly, Category = "Countdown")
    FLinearColor StartColor = FLinearColor::White;

    // Colour at FIGHT! — blood red
    UPROPERTY(EditDefaultsOnly, Category = "Countdown")
    FLinearColor EndColor = FLinearColor(0.55f, 0.02f, 0.02f);

    int32 StartSeconds = 10;
};
