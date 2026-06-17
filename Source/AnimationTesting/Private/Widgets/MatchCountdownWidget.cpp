// Christopher Naglik All Rights Reserved

#include "Widgets/MatchCountdownWidget.h"
#include "Components/TextBlock.h"

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
}