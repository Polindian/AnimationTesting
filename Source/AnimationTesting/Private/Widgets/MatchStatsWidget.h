// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/MatchStatsTypes.h"
#include "MatchStatsWidget.generated.h"

class APlayerState;

UCLASS()
class UMatchStatsWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void ShowStats(class APlayerState* LocalPlayerState);

private:
    // Gold panel = MVP (everyone sees the same), silver = this client
    UPROPERTY(meta = (BindWidget))
    class UMatchStatsPanel* MVPPanel;

    UPROPERTY(meta = (BindWidget))
    class UMatchStatsPanel* PlayerPanel;
};