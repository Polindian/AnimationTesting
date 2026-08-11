// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GameplayTagContainer.h"
#include "ChrisSoundLibrary.h"
#include "ChrisSoundCueNotify.generated.h"


/**
 * Plays a sound from the sound library when its gameplay cue fires.
 * Cues execute on every relevant client, so OwnerOnly2D exists for sounds
 * that should stay personal to the target.
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

	UPROPERTY(EditDefaultsOnly, Category = "Chris Sound")
	EChrisCueSoundMode Mode = EChrisCueSoundMode::OwnerOnly2D;

	/** Checks the target character's voice library before the global one. 2D mode only. */
	UPROPERTY(EditDefaultsOnly, Category = "Chris Sound")
	bool bUseCharacterVoice = true;
};