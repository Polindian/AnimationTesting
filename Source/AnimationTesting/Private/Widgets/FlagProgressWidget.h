// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Framework/Flag.h"
#include "FlagProgressWidget.generated.h"

UCLASS()
class UFlagProgressWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    UPROPERTY(meta = (BindWidget))
    class UImage* ProgressImage;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TeamOneInfluence;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TeamTwoInfluence;

    UPROPERTY(meta = (BindWidget))
    class UBorder* ProgressBorder;

    UPROPERTY(meta = (BindWidget))
    class UImage* BlueFlagCapture;

    UPROPERTY(meta = (BindWidget))
    class UImage* RedFlagCapture;

    // All flags in the level
    UPROPERTY()
    TArray<AFlag*> AllFlags;

    // The flag the local player is currently inside 
    UPROPERTY()
    AFlag* ActiveFlag = nullptr;

    // Checks which flag (if any) the local player is inside
    AFlag* FindLocalPlayerFlag() const;

    UPROPERTY()
    class AFlag* FlagZone;

    UPROPERTY()
    UMaterialInstanceDynamic* ProgressMaterial;

    float DisplayedProgress = 0.5f;

    // To move toward on influence update
    float TargetProgress = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Progress Bar")
    float ProgressInterpSpeed = 3.f;

    void OnTeamInfluenceChanged(float TeamOneRate, float TeamTwoRate);

    void OnFlagCaptured(EFlagOwnership WinningTeam);
};