// Christopher Naglik All Rights Reserved


#include "Player/MainMenuPlayerController.h"
#include "Audio/ChrisAudioSubsystem.h"
#include "Audio/ChrisGameplayTags.h"

AMainMenuPlayerController::AMainMenuPlayerController()
{
	bAutoManageActiveCameraTarget = false;
}

void AMainMenuPlayerController::StartMenuMusic()
{
	// Called by the widget once the intro video is done, so the two don't overlap
	if (MenuMusic) { return; }

	if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(this))
	{
		MenuMusic = Audio->PlayLooping2DFadeIn(ChrisGameplayTags::Audio_Music_MainMenu, MusicFadeInTime);
	}
}


void AMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

}

// Travelling to the lobby tears this controller down, so the track stops with it
void AMainMenuPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UChrisAudioSubsystem::StopLoopingSound(MenuMusic, MusicFadeOutTime);

	Super::EndPlay(EndPlayReason);
}