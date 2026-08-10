// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ChrisAudioSettings.generated.h"

class UChrisSoundLibrary;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Chris Audio"))
class ANIMATIONTESTING_API UChrisAudioSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UChrisAudioSettings();

	UPROPERTY(Config, EditAnywhere, Category = "Audio")
	TSoftObjectPtr<UChrisSoundLibrary> SoundLibrary;

	UPROPERTY(Config, EditAnywhere, Category = "Audio")
	TSoftObjectPtr<class USoundControlBus> MasterBus;

	UPROPERTY(Config, EditAnywhere, Category = "Audio")
	TSoftObjectPtr<class USoundControlBus> MusicBus;

	UPROPERTY(Config, EditAnywhere, Category = "Audio")
	TSoftObjectPtr<class USoundControlBus> SFXBus;
};