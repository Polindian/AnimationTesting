// Christopher Naglik All Rights Reserved


#include "LoadingScreenWidget.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"
#include "Framework/ChrisGameInstance.h"

void ULoadingScreenWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	FWidgetAnimationDynamicEvent FadeFinished;
	FadeFinished.BindDynamic(this, &ULoadingScreenWidget::HandleFadeOutFinished);
	BindToAnimationFinished(Anim_FadeOut, FadeFinished);
}

void ULoadingScreenWidget::ShowRandomHint()
{
	if (Hints.Num() == 0 || !HintText) { return; }

	UChrisGameInstance* GI = GetGameInstance<UChrisGameInstance>();

	int32 Index = FMath::RandRange(0, Hints.Num() - 1);

	// Nudge off the last one — pure random repeats often enough to be noticed
	if (GI && Hints.Num() > 1 && Index == GI->LastHintIndex)
	{
		Index = (Index + 1) % Hints.Num();
	}

	if (GI) { GI->LastHintIndex = Index; }

	HintText->SetText(Hints[Index]);
}

void ULoadingScreenWidget::DismissWithFade()
{
	if (Anim_FadeOut)
	{
		PlayAnimation(Anim_FadeOut);
	}
	else
	{
		RemoveFromParent();
	}
}

void ULoadingScreenWidget::HandleFadeOutFinished()
{
	RemoveFromParent();
}