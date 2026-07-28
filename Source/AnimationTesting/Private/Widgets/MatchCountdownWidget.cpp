// Christopher Naglik All Rights Reserved

#include "Widgets/MatchCountdownWidget.h"
#include "Components/TextBlock.h"

void UMatchCountdownWidget::StartCountdown(int32 TotalSeconds)
{
    StartSeconds = FMath::Max(1, TotalSeconds);   // guard against divide-by-zero
    UpdateCountdown(TotalSeconds);
}

void UMatchCountdownWidget::UpdateCountdown(int32 SecondsRemaining)
{
    if (!CountdownText) return;

    if (SecondsRemaining > 0)
    {
        // Show the number: "5", "4", "3", "2", "1"
        CountdownText->SetText(FText::AsNumber(SecondsRemaining));
    }
    else
    {
        // 0 means "FIGHT!"
        CountdownText->SetText(FText::FromString(TEXT("FIGHT!")));
    }

    // Alpha runs 0 at the start of the countdown to 1 at FIGHT!, so the text bleeds from white toward red as the round approaches
    const float Alpha = 1.f - ((float)FMath::Max(0, SecondsRemaining) / (float)StartSeconds);
    CountdownText->SetColorAndOpacity(FSlateColor(FMath::Lerp(StartColor, EndColor, Alpha)));
}