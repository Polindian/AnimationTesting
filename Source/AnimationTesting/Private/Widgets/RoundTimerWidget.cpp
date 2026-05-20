// Christopher Naglik All Rights Reserved

#include "Widgets/RoundTimerWidget.h"
#include "Components/TextBlock.h"

void URoundTimerWidget::StartTimer(float Duration)
{
    // Store the absolute time when the timer expires.
    // Example: if current time is 120.0 and duration is 60, EndTime = 180.0
    EndTime = GetWorld()->GetTimeSeconds() + Duration;
}

void URoundTimerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!TimerText) return;

    // Calculate how many seconds remain. Clamp to 0 so it never goes negative.
    float Remaining = FMath::Max(0.f, EndTime - GetWorld()->GetTimeSeconds());

    // Truncate decimals and change to 'minutes:seconds' format
    int32 Minutes = (int32)(Remaining / 60.f);
    int32 Seconds = (int32)(Remaining) % 60;

    FString TimeString = FString::Printf(TEXT("%d:%02d"), Minutes, Seconds);
    TimerText->SetText(FText::FromString(TimeString));
}