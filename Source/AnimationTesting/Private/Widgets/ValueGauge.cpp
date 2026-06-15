// Christopher Naglik All Rights Reserved

#include "Widgets/ValueGauge.h"
#include "Components/ProgressBar.h"
#include "AbilitySystemComponent.h"
#include "Components/TextBlock.h"

void UValueGauge::NativePreConstruct()
{
	Super::NativePreConstruct();
	ProgressBar->SetFillColorAndOpacity(BarColor);

	ValueText->SetFont(ValueTextFont);
	ProgressBar->SetVisibility(bProgressBarVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

	if (DamageBar)
	{
		// Yellow-orange color for the damage trail
		DamageBar->SetFillColorAndOpacity(FLinearColor(0.65f, 0.35f, 0.0f, 1.0f));
		DamageBar->SetPercent(1.f);
		DamageBar->SetVisibility(bProgressBarVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

		// Make the main bar's background transparent so the DamageBar shows through
		FProgressBarStyle Style = ProgressBar->GetWidgetStyle();
		Style.BackgroundImage.TintColor = FSlateColor(FLinearColor::Transparent);
		ProgressBar->SetWidgetStyle(Style);
	}
}

void UValueGauge::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!DamageBar || !bDamageBarEnabled || DamageBarPercent <= TargetPercent)
	{
		return;
	}

	if (DrainDelayRemaining > 0.f)
	{
		DrainDelayRemaining -= InDeltaTime;
		return;
	}

	DamageBarPercent = FMath::FInterpConstantTo(DamageBarPercent, TargetPercent, InDeltaTime, DrainSpeed);

	if (FMath::IsNearlyEqual(DamageBarPercent, TargetPercent, 0.001f))
	{
		DamageBarPercent = TargetPercent;
	}

	DamageBar->SetPercent(DamageBarPercent);
}

void UValueGauge::SetAndBoundGameplayAttribute(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayAttribute& Attribute, const FGameplayAttribute& MaxAttribute)
{
	if (AbilitySystemComponent)
	{
		bool bFound;
		float Value = AbilitySystemComponent->GetGameplayAttributeValue(Attribute, bFound);
		float MaxValue = AbilitySystemComponent->GetGameplayAttributeValue(MaxAttribute, bFound);

		if (bFound)
		{
			SetValue(Value, MaxValue);

			// Initialize damage bar to match (no yellow trail on spawn)
			if (DamageBar && MaxValue > 0.f)
			{
				DamageBarPercent = Value / MaxValue;
				TargetPercent = DamageBarPercent;
				DamageBar->SetPercent(DamageBarPercent);
			}
		}

		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &UValueGauge::ValueChanged);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(MaxAttribute).AddUObject(this, &UValueGauge::MaxValueChanged);
	}
}

void UValueGauge::SetValue(float NewValue, float NewMaxValue)
{
	CachedValue = NewValue;
	CachedMaxValue = NewMaxValue;

	if (NewMaxValue == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Value Gauge: %s, NewMaxValue cannot be 0"), *GetName());
		return;
	}

	float NewPercent = NewValue / NewMaxValue;

	// Main bar always snaps immediately
	ProgressBar->SetPercent(NewPercent);

	if (DamageBar)
	{
		if (bDamageBarEnabled)
		{
			// Damage trail is active (health bar)
			if (NewPercent < TargetPercent)
			{
				// Damage taken — freeze DamageBar where it is, reset drain delay
				DrainDelayRemaining = DrainDelay;
			}
			else if (NewPercent > DamageBarPercent)
			{
				// Healing — snap damage bar up immediately (no yellow trail for heals)
				DamageBarPercent = NewPercent;
				DamageBar->SetPercent(DamageBarPercent);
			}
		}
		else
		{
			// Damage trail disabled (mana bar) — just snap to match
			DamageBarPercent = NewPercent;
			DamageBar->SetPercent(NewPercent);
		}

		TargetPercent = NewPercent;
	}

	FNumberFormattingOptions FormatOptions = FNumberFormattingOptions().SetMaximumFractionalDigits(0);

	ValueText->SetText(
		FText::Format(
			FTextFormat::FromString("{0}/{1}"),
			FText::AsNumber(NewValue, &FormatOptions),
			FText::AsNumber(NewMaxValue, &FormatOptions)
		)
	);
}

void UValueGauge::SetBarColor(FLinearColor NewColor)
{
	BarColor = NewColor;
	if (ProgressBar)
	{
		ProgressBar->SetFillColorAndOpacity(NewColor);
	}
}

void UValueGauge::SetTextVisibility(bool bVisible)
{
	if (ValueText)
	{
		ValueText->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}
}

void UValueGauge::SetDamageBarEnabled(bool bEnabled)
{
	bDamageBarEnabled = bEnabled;
}

void UValueGauge::ValueChanged(const FOnAttributeChangeData& ChangedData)
{
	SetValue(ChangedData.NewValue, CachedMaxValue);
}

void UValueGauge::MaxValueChanged(const FOnAttributeChangeData& ChangedData)
{
	SetValue(CachedValue, ChangedData.NewValue);
}