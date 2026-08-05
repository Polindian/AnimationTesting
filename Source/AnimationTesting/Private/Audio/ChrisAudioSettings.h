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
};