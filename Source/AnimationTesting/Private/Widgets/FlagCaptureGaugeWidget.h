// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Framework/Flag.h"
#include "FlagCaptureGaugeWidget.generated.h"

// Circular progress gauge showing total flag capture percentage
// Follows the same pattern as LevelGaugeWidget — a UImage with a 
// dynamic material driven by a "Percent" scalar parameter
UCLASS()
class UFlagCaptureGaugeWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    UPROPERTY(meta = (BindWidget))
    class UImage* CaptureProgressImage;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* ZoneLabel;

    UPROPERTY()
    class AFlag* FlagZone;

    UPROPERTY(EditInstanceOnly, Category = "Capture")
    int32 TargetZoneID = 0;

    UPROPERTY()
    UMaterialInstanceDynamic* CaptureMaterial;

    UPROPERTY(EditDefaultsOnly, Category = "Material")
    FName PercentParamName = "Percent";

    UPROPERTY(EditDefaultsOnly, Category = "Material")
    FName ColorParamName = "GaugeColor";

    // Smooth interpolation 
    float DisplayedPercent = 0.f;
    float TargetPercent = 0.f;

    UPROPERTY(EditDefaultsOnly, Category = "Visual")
    float InterpSpeed = 5.f;

    // Team colors for the gauge
    UPROPERTY(EditDefaultsOnly, Category = "Visual")
    FLinearColor TeamOneColor = FLinearColor(1.f, 0.1f, 0.1f, 1.f);  // Red

    UPROPERTY(EditDefaultsOnly, Category = "Visual")
    FLinearColor TeamTwoColor = FLinearColor(0.1f, 0.1f, 1.f, 1.f);  // Blue

    UPROPERTY(EditDefaultsOnly, Category = "Visual")
    FLinearColor NeutralColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.f); // Grey

    // Tracks which team color is currently displayed
    EFlagOwnership CurrentDisplayedOwnership = EFlagOwnership::Neutral;

    FNumberFormattingOptions NumberFormat;
};