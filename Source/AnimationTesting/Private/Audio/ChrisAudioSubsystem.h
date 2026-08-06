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

	/** Non-positional. Only ever heard on the machine that calls it. */
	UFUNCTION(BlueprintCallable, Category = "Chris|Audio")
	void Play2D(FGameplayTag Tag);

	/** Positional one-shot at a fixed world point. */
	UFUNCTION(BlueprintCallable, Category = "Chris|Audio")
	void PlayAtLocation(FGameplayTag Tag, FVector Location);

	/** Positional and follows the component. Use for anything on a moving actor. */
	UFUNCTION(BlueprintCallable, Category = "Chris|Audio")
	UAudioComponent* PlayAttached(FGameplayTag Tag, USceneComponent* AttachTo, FName SocketName = NAME_None);

	UFUNCTION(BlueprintPure, Category = "Chris|Audio", meta = (WorldContext = "WorldContextObject"))
	static UChrisAudioSubsystem* Get(const UObject* WorldContextObject);

private:
	UPROPERTY(Transient)
	TObjectPtr<UChrisSoundLibrary> Library = nullptr;
};