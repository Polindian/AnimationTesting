// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/MatchStatsTypes.h"
#include "MatchStatsWidget.generated.h"

class APlayerState;
class UWidgetAnimation;

UCLASS()
class UMatchStatsWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void ShowStats(APlayerState* LocalPlayerState);

    // Fired when LEAVE MATCH is pressed — the owner handles the travel
    FSimpleMulticastDelegate OnLeaveMatchRequested;

    void PlayLeaveFade(float Duration);

protected:
    virtual void NativeOnInitialized() override;

private:
    // Gold panel = MVP (everyone sees the same), silver = this client
    UPROPERTY(meta = (BindWidget))
    class UMatchStatsPanel* MVPPanel;

    UPROPERTY(meta = (BindWidget))
    class UMatchStatsPanel* PlayerPanel;

    UPROPERTY(meta = (BindWidget))
    class UMenuButtonWidget* LeaveMatchButton;

    // Whole screen fades up together — black background and panels
    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> Anim_FadeIn;

    UFUNCTION()
    void HandleFadeInFinished();

    UFUNCTION()
    void HandleLeaveMatchClicked();

    UPROPERTY(meta = (BindWidget))
    class UWidget* ContentRoot;

    FTimerHandle LeaveFadeTimerHandle;
};