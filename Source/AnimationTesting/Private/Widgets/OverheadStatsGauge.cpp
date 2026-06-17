// Christopher Naglik All Rights Reserved


#include "OverheadStatsGauge.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Widgets/ValueGauge.h"
#include "GAS/ChrisAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "GenericTeamAgentInterface.h"

void UOverheadStatsGauge::ConfigureWithASC(UAbilitySystemComponent* AbilitySystemComponent)
{
	
	if (AbilitySystemComponent)
	{
		HealthBar->SetAndBoundGameplayAttribute(AbilitySystemComponent, UChrisAttributeSet::GetHealthAttribute(), UChrisAttributeSet::GetMaxHealthAttribute());
        HealthBar->SetDamageBarEnabled(true);

		ManaBar->SetAndBoundGameplayAttribute(AbilitySystemComponent, UChrisAttributeSet::GetManaAttribute(), UChrisAttributeSet::GetMaxManaAttribute());
	}
}

void UOverheadStatsGauge::ConfigureTeamColor(AActor* OwnerActor)
{
    if (!OwnerActor) return;

    IGenericTeamAgentInterface* OwnerTeam = Cast<IGenericTeamAgentInterface>(OwnerActor);

    APlayerController* LocalPC = UGameplayStatics::GetPlayerController(this, 0);
    APawn* LocalPawn = LocalPC ? LocalPC->GetPawn() : nullptr;
    IGenericTeamAgentInterface* LocalTeam = Cast<IGenericTeamAgentInterface>(LocalPawn);

    if (OwnerTeam && LocalTeam)
    {
        // ======================================================
        // Compare raw team IDs 
        // ======================================================
        FGenericTeamId OwnerID = OwnerTeam->GetGenericTeamId();
        FGenericTeamId LocalID = LocalTeam->GetGenericTeamId();

        if (OwnerID == LocalID)
        {
            // Same team — green
            HealthBar->SetBarColor(FLinearColor(0.0f, 0.8f, 0.15f, 1.0f));
        }
        else
        {
            // Enemy — red
            HealthBar->SetBarColor(FLinearColor(0.9f, 0.1f, 0.1f, 1.0f));
        }
    }
}

void UOverheadStatsGauge::SetTextVisibility(bool bVisible)
{
    ESlateVisibility Vis = bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
    if (HealthBar)
    {
        HealthBar->SetTextVisibility(bVisible);
    }
    if (ManaBar)
    {
        ManaBar->SetTextVisibility(bVisible);
    }
}

