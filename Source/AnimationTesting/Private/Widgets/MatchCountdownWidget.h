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
    // Called by PlayerController to update the displayed number.
    void UpdateCountdown(int32 SecondsRemaining);

private:
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* CountdownText;
};
