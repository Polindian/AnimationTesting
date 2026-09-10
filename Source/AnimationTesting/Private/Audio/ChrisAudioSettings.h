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

	UPROPERTY(Config, EditAnywhere, Category = "Audio")
	TSoftObjectPtr<class USoundSubmix> MusicSubmix;

	UPROPERTY(Config, EditAnywhere, Category = "Audio")
	TArray<TSoftObjectPtr<class USoundEffectSubmixPreset>> DeathEffectChain;

	UPROPERTY(Config, EditAnywhere, Category = "Audio")
	float DeathEffectFadeTime = 0.f;

	// Neutral chain swapped in on respawn; avoids the fade-to-silence of clearing
	UPROPERTY(Config, EditAnywhere, Category = "Audio")
	TArray<TSoftObjectPtr<class USoundEffectSubmixPreset>> OpenEffectChain;
};