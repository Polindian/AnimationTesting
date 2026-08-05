// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ChrisSoundLibrary.generated.h"

class USoundBase;
class USoundAttenuation;
class USoundConcurrency;

/** Everything about how one sound plays, editable in the sound library asset. */
USTRUCT(BlueprintType)
struct FChrisSoundDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> Sound = nullptr;

	UPROPERTY(EditAnywhere, Category = "Sound", meta = (UIMin = "0.0", UIMax = "2.0"))
	float VolumeMultiplier = 1.f;

	UPROPERTY(EditAnywhere, Category = "Sound", meta = (UIMin = "0.5", UIMax = "2.0"))
	float PitchMultiplier = 1.f;

	/** Only read by the 3D play paths. Leave empty for 2D sounds. */
	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundAttenuation> Attenuation = nullptr;

	/** Voice limiting. UI entries all share the "max 1, stop oldest" asset. */
	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundConcurrency> Concurrency = nullptr;
};

/** The single place that maps audio.* tags to sounds. One asset per project. */
UCLASS(BlueprintType)
class ANIMATIONTESTING_API UChrisSoundLibrary : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Sounds", meta = (ForceInlineRow, Categories = "audio"))
	TMap<FGameplayTag, FChrisSoundDef> Sounds;

	/** Exact match first, then walks up the tag tree to the nearest filled-in parent. */
	const FChrisSoundDef* FindSound(const FGameplayTag& Tag) const;
};