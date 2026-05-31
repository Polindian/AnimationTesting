// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "GameplayMenu.generated.h"

UCLASS()
class UGameplayMenu : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeConstruct() override;
    FOnButtonClickedEvent& GetReturnToArenaButtonClickedEventDelegate();

private:
    UPROPERTY(meta = (BindWidget))
    class UButton* ReturnToArenaButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* SettingsButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* LeaveMatchButton;

    UPROPERTY(meta = (BindWidget))
    class URetainerBox* ReturnToArenaRetainer;

    UPROPERTY(meta = (BindWidget))
    class URetainerBox* SettingsRetainer;

    UPROPERTY(meta = (BindWidget))
    class URetainerBox* LeaveMatchRetainer;

    UPROPERTY(EditDefaultsOnly, Category = "Menu Style")
    UMaterialInterface* HoverGradientMaterial;

    void SetRetainerHovered(URetainerBox* Retainer, bool bHovered);

    UFUNCTION()
    void OnReturnToArenaHovered();
    UFUNCTION()
    void OnReturnToArenaUnhovered();
    UFUNCTION()
    void OnSettingsHovered();
    UFUNCTION()
    void OnSettingsUnhovered();
    UFUNCTION()
    void OnLeaveMatchHovered();
    UFUNCTION()
    void OnLeaveMatchUnhovered();

    UFUNCTION()
    void LeaveMatch();

    UFUNCTION()
    void GoSettingsPage();
};