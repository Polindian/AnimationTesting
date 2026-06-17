// Christopher Naglik All Rights Reserved

#include "Widgets/FlagProgressWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Framework/Flag.h"
#include "Kismet/GameplayStatics.h"

void UFlagProgressWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Fallback: find flags if they weren't available during NativeConstruct
    if (AllFlags.IsEmpty())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(this, AFlag::StaticClass(), FoundActors);
        for (AActor* Actor : FoundActors)
        {
            if (AFlag* Flag = Cast<AFlag>(Actor))
            {
                AllFlags.Add(Flag);
            }
        }
    }

    if (ProgressImage)
    {
        ProgressMaterial = ProgressImage->GetDynamicMaterial();
    }

    // Start hidden — only shows when player enters a zone
    ProgressBorder->SetVisibility(ESlateVisibility::Collapsed);

    if (BlueFlagCapture)
        BlueFlagCapture->SetVisibility(ESlateVisibility::Hidden);
    if (RedFlagCapture)
        RedFlagCapture->SetVisibility(ESlateVisibility::Hidden);
}

void UFlagProgressWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Fallback: find flags if they weren't ready during NativeConstruct
    if (AllFlags.IsEmpty())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(this, AFlag::StaticClass(), FoundActors);
        for (AActor* Actor : FoundActors)
        {
            if (AFlag* Flag = Cast<AFlag>(Actor))
            {
                AllFlags.Add(Flag);
            }
        }
    }

    // Check if local player is inside any flag zone
    AFlag* CurrentFlag = FindLocalPlayerFlag();

    // Player entered a new zone or switched zones
    if (CurrentFlag != ActiveFlag)
    {
        ActiveFlag = CurrentFlag;

        if (ActiveFlag)
        {
            ProgressBorder->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

            // Reset progress bar state for new zone
            DisplayedProgress = 0.5f;
            TargetProgress = 0.5f;

            // Reset flag capture images
            if (BlueFlagCapture)
                BlueFlagCapture->SetVisibility(ESlateVisibility::Hidden);
            if (RedFlagCapture)
                RedFlagCapture->SetVisibility(ESlateVisibility::Hidden);
            if (TeamOneInfluence)
                TeamOneInfluence->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
            if (TeamTwoInfluence)
                TeamTwoInfluence->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        }
        else
        {
            // Player left all zones — hide the progress bar
            ProgressBorder->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    // If we have an active flag, update the display from its replicated values
    if (ActiveFlag)
    {
        float T1Rate = ActiveFlag->GetTeamOneCaptureRate();
        float T2Rate = ActiveFlag->GetTeamTwoCaptureRate();

        // Update text
        if (TeamOneInfluence)
            TeamOneInfluence->SetText(FText::AsNumber(FMath::RoundToInt(T1Rate)));
        if (TeamTwoInfluence)
            TeamTwoInfluence->SetText(FText::AsNumber(FMath::RoundToInt(T2Rate)));

        // Calculate target progress (bar ratio)
        float Total = T1Rate + T2Rate;
        if (Total > 0.f)
        {
            TargetProgress = T2Rate / Total;
        }
        else
        {
            TargetProgress = 0.5f;
        }

        // Handle captured state
        if (ActiveFlag->IsCaptured())
        {
            if (TeamOneInfluence)
                TeamOneInfluence->SetVisibility(ESlateVisibility::Hidden);
            if (TeamTwoInfluence)
                TeamTwoInfluence->SetVisibility(ESlateVisibility::Hidden);

            if (ActiveFlag->GetOwnership() == EFlagOwnership::TeamOne)
            {
                TargetProgress = 0.f;
                if (RedFlagCapture)
                    RedFlagCapture->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
            }
            else if (ActiveFlag->GetOwnership() == EFlagOwnership::TeamTwo)
            {
                TargetProgress = 1.f;
                if (BlueFlagCapture)
                    BlueFlagCapture->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
            }
        }

        // Smooth interpolation
        DisplayedProgress = FMath::FInterpTo(DisplayedProgress, TargetProgress, InDeltaTime, ProgressInterpSpeed);

        if (ProgressMaterial)
        {
            ProgressMaterial->SetScalarParameterValue("Progress", DisplayedProgress);
        }
    }
}

AFlag* UFlagProgressWidget::FindLocalPlayerFlag() const
{
    APlayerController* PC = GetOwningPlayer();

    if (!PC)
    {
        PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    }
    if (!PC)
        return nullptr;

    APawn* Pawn = PC->GetPawn();
    if (!Pawn)
    {
        return nullptr;
    }

    FVector PawnLocation = Pawn->GetActorLocation();

    for (AFlag* Flag : AllFlags)
    {
        if (!Flag)
            continue;

        float Distance = FVector::Dist(PawnLocation, Flag->GetActorLocation());
        float Radius = Flag->GetInfluenceRadius();

        if (Distance <= Radius)
        {
            return Flag;
        }
    }

    return nullptr;
}