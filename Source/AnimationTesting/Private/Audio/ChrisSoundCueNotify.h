// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GameplayTagContainer.h"
#include "ChrisSoundCueNotify.generated.h"

/**
 * Plays a sound from the sound library when its cue fires.
 * Cues replicate to every client, so bOwnerOnly2D restricts playback to the
 * target's own machine for sounds that should be personal.
 */
UCLASS()
class ANIMATIONTESTING_API UChrisSoundCueNotify : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Chris Sound", meta = (Categories = "audio"))
	FGameplayTag SoundTag;

	/** Only the target hears it, non-positional. Untick for world sounds everyone hears. */
	UPROPERTY(EditDefaultsOnly, Category = "Chris Sound")
	bool bOwnerOnly2D = true;

	/** Checks the target character's voice library before the global one */
	UPROPERTY(EditDefaultsOnly, Category = "Chris Sound")
	bool bUseCharacterVoice = true;
};