// Christopher Naglik All Rights Reserved

#include "Widgets/FlagCaptureGaugeWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Framework/Flag.h"
#include "Kismet/GameplayStatics.h"

void UFlagCaptureGaugeWidget::NativeConstruct()
{
    Super::NativeConstruct();
    NumberFormat.SetMaximumFractionalDigits(0);

    // Find the specific flag matching our ZoneID
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(this, AFlag::StaticClass(), FoundActors);
    for (AActor* Actor : FoundActors)
    {
        AFlag* Flag = Cast<AFlag>(Actor);
        if (Flag && Flag->GetZoneID() == TargetZoneID)
        {
            FlagZone = Flag;
            break;
        }
    }

    if (CaptureProgressImage)
    {
        CaptureMaterial = CaptureProgressImage->GetDynamicMaterial();
    }

    if (CaptureMaterial)
    {
        CaptureMaterial->SetScalarParameterValue(PercentParamName, 0.f);
        CaptureMaterial->SetVectorParameterValue(ColorParamName, NeutralColor);
    }

    if (ZoneLabel)
    {
        ZoneLabel->SetText(FText::Format(NSLOCTEXT("Flag", "ZoneLabel", "Z{0}"), FText::AsNumber(TargetZoneID)));
    }
}

void UFlagCaptureGaugeWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!FlagZone)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(this, AFlag::StaticClass(), FoundActors);
        for (AActor* Actor : FoundActors)
        {
            AFlag* Flag = Cast<AFlag>(Actor);
            if (Flag && Flag->GetZoneID() == TargetZoneID)
            {
                FlagZone = Flag;
                break;
            }
        }
        if (!FlagZone)
            return;
    }

    if (!CaptureMaterial && CaptureProgressImage)
    {
        CaptureMaterial = CaptureProgressImage->GetDynamicMaterial();
    }
    if (!CaptureMaterial)
        return;

    TargetPercent = FlagZone->GetCapturePercent() / 100.f;
    EFlagOwnership CurrentOwnership = FlagZone->GetOwnership();

    // Smoothly interpolate the displayed value toward the target
    DisplayedPercent = FMath::FInterpTo(DisplayedPercent, TargetPercent, InDeltaTime, InterpSpeed);

    // Update the material's radial fill
    if (CaptureMaterial)
    {
        CaptureMaterial->SetScalarParameterValue(PercentParamName, DisplayedPercent);

        // Update the colour if ownership changed
        if (CurrentOwnership != CurrentDisplayedOwnership)
        {
            CurrentDisplayedOwnership = CurrentOwnership;

            FLinearColor NewColor;
            switch (CurrentOwnership)
            {
            case EFlagOwnership::TeamOne:
                NewColor = TeamOneColor;
                break;
            case EFlagOwnership::TeamTwo:
                NewColor = TeamTwoColor;
                break;
            default:
                NewColor = NeutralColor;
                break;
            }

            CaptureMaterial->SetVectorParameterValue(ColorParamName, NewColor);
        }

        // Update zone label when captured (separate from gauge color)
        if (ZoneLabel && FlagZone->IsCaptured() && CurrentOwnership != EFlagOwnership::Neutral)
        {
            FLinearColor LabelColor = (CurrentOwnership == EFlagOwnership::TeamOne) ? TeamOneColor : TeamTwoColor;
            ZoneLabel->SetColorAndOpacity(FSlateColor(LabelColor));
        }
        else if (ZoneLabel && !FlagZone->IsCaptured())
        {
            ZoneLabel->SetColorAndOpacity(FSlateColor(FLinearColor::White));
        }
    }
}