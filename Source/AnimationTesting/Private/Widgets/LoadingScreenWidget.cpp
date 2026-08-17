// Christopher Naglik All Rights Reserved


#include "LoadingScreenWidget.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"
#include "Framework/ChrisGameInstance.h"
#include "Audio/ChrisAudioSubsystem.h"
#include "Components/AudioComponent.h"
#include "Audio/ChrisGameplayTags.h"

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
	UChrisAudioSubsystem::StopLoopingSound(LoadingAudio, AudioFadeOutTime);

	if (Anim_FadeOut)
	{
		PlayAnimation(Anim_FadeOut);
	}
	else
	{
		RemoveFromParent();
	}
}

void ULoadingScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (LoadingAudio) { return; }

	if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
	{
		LoadingAudio = Audio->PlayLooping2DFadeIn(
			ChrisGameplayTags::Audio_Ambience_LoadingScreen, AudioFadeInTime);
	}

	if (Anim_ZoomIn)
	{
		PlayAnimation(Anim_ZoomIn);
	}
}

void ULoadingScreenWidget::NativeDestruct()
{
	// Safety net for any path that removes the widget without dismissing it
	UChrisAudioSubsystem::StopLoopingSound(LoadingAudio, 0.f);

	Super::NativeDestruct();
}

void ULoadingScreenWidget::HandleFadeOutFinished()
{
	RemoveFromParent();
}