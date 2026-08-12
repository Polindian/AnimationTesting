// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "ChrisAudioSubsystem.generated.h"

class UChrisSoundLibrary;
class UAudioComponent;
class USceneComponent;

UCLASS()
class ANIMATIONTESTING_API UChrisAudioSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Positional one-shot at a fixed world point. */
	UFUNCTION(BlueprintCallable, Category = "Chris|Audio")
	void PlayAtLocation(FGameplayTag Tag, FVector Location);

	/** Positional and follows the component. Use for anything on a moving actor. */
	UFUNCTION(BlueprintCallable, Category = "Chris|Audio")
	UAudioComponent* PlayAttached(FGameplayTag Tag, USceneComponent* AttachTo, FName SocketName = NAME_None);

	UFUNCTION(BlueprintPure, Category = "Chris|Audio", meta = (WorldContext = "WorldContextObject"))
	static UChrisAudioSubsystem* Get(const UObject* WorldContextObject);

	/** Pushes the current saved volumes to the audio engine. Call on startup and after any slider change. */
	UFUNCTION(BlueprintCallable, Category = "Chris|Audio")
	void ApplyVolumeSettings();


	/** Non-positional. Only ever heard on the machine that calls it. */
	UFUNCTION(BlueprintCallable, Category = "Chris|Audio")
	void Play2D(FGameplayTag Tag, UChrisSoundLibrary* OverrideLibrary = nullptr);

	/** Returns a component for looping sounds. */
	UFUNCTION(BlueprintCallable, Category = "Chris|Audio")
	class UAudioComponent* PlayLooping2D(FGameplayTag Tag, UChrisSoundLibrary* OverrideLibrary = nullptr);


	/** Looping 2D with a fade in. Caller owns the component and must stop it. */
	UFUNCTION(BlueprintCallable, Category = "Chris|Audio")
	class UAudioComponent* PlayLooping2DFadeIn(FGameplayTag Tag, float FadeInTime);

	/** Fades out and destroys. Safe with null. */
	UFUNCTION(BlueprintCallable, Category = "Chris|Audio")
	static void StopLoopingSound(class UAudioComponent*& Component, float FadeOutTime);

private:
	UPROPERTY(Transient)
	TObjectPtr<UChrisSoundLibrary> Library = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<class USoundControlBus> MasterBus = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<class USoundControlBus> MusicBus = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<class USoundControlBus> SFXBus = nullptr;
};