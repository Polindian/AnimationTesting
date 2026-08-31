// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "ChrisGameUserSettings.generated.h"

UCLASS()
class ANIMATIONTESTING_API UChrisGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	// Convenience accessor so callers don't have to cast the engine singleton
	UFUNCTION(BlueprintPure, Category = "Chris|Settings")
	static UChrisGameUserSettings* GetChrisSettings();

	UFUNCTION(BlueprintPure, Category = "Chris|Audio")
	float GetMasterVolume() const { return MasterVolume; }

	UFUNCTION(BlueprintPure, Category = "Chris|Audio")
	float GetMusicVolume() const { return MusicVolume; }

	UFUNCTION(BlueprintPure, Category = "Chris|Audio")
	float GetSFXVolume() const { return SFXVolume; }

	// Setters only store the value — applying it is the audio subsystem's job
	UFUNCTION(BlueprintCallable, Category = "Chris|Audio")
	void SetMasterVolume(float InVolume) { MasterVolume = FMath::Clamp(InVolume, 0.f, 1.f); }

	UFUNCTION(BlueprintCallable, Category = "Chris|Audio")
	void SetMusicVolume(float InVolume) { MusicVolume = FMath::Clamp(InVolume, 0.f, 1.f); }

	UFUNCTION(BlueprintCallable, Category = "Chris|Audio")
	void SetSFXVolume(float InVolume) { SFXVolume = FMath::Clamp(InVolume, 0.f, 1.f); }

	UFUNCTION(BlueprintPure, Category = "Chris|Graphics")
	int32 GetGraphicsQuality() const { return GetOverallScalabilityLevel(); }

	// Applies immediately — moves view distance, shadows, textures, effects and AA together, and persists to GameUserSettings.ini
	UFUNCTION(BlueprintCallable, Category = "Chris|Graphics")
	void SetGraphicsQuality(int32 Level);


private:
	// Config: saved to and loaded from GameUserSettings.ini automatically
	UPROPERTY(Config)
	float MasterVolume = 1.f;

	UPROPERTY(Config)
	float MusicVolume = 0.7f;

	UPROPERTY(Config)
	float SFXVolume = 1.f;

	UPROPERTY(Config)
	int32 GraphicsQuality = 2;


};