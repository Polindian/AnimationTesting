// Christopher Naglik All Rights Reserved


#include "ChrisSoundCueNotify.h"
#include "ChrisAudioSubsystem.h"
#include "ChrisSoundLibrary.h"
#include "Character/ChrisCharacter.h"
#include "GameFramework/Pawn.h"

bool UChrisSoundCueNotify::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget || !SoundTag.IsValid()) { return false; }

	UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(MyTarget);
	if (!Audio) { return false; }

	switch (Mode)
	{
	case EChrisCueSoundMode::OwnerOnly2D:
	{
		// Cues execute on every client. IsPlayerControlled excludes AI, which
		// counts as locally controlled on the server.
		const APawn* Pawn = Cast<APawn>(MyTarget);
		if (!Pawn || !Pawn->IsLocallyControlled() || !Pawn->IsPlayerControlled())
		{
			return false;
		}

		AChrisCharacter* Character = Cast<AChrisCharacter>(MyTarget);
		UChrisSoundLibrary* Voice = (bUseCharacterVoice && Character)
			? Character->GetVoiceLibrary()
			: nullptr;

		Audio->Play2D(SoundTag, Voice);
		break;
	}

	case EChrisCueSoundMode::WorldAtLocation:
	{
		// Parameters carries the impact point when the cue was fired with target
		// data; otherwise fall back to the actor's own position
		FVector Location = MyTarget->GetActorLocation();
		if (!Parameters.Location.IsNearlyZero())
		{
			Location = FVector(Parameters.Location);
		}

		Audio->PlayAtLocation(SoundTag, Location);
		break;
	}

	case EChrisCueSoundMode::WorldAttached:
	{
		Audio->PlayAttached(SoundTag, MyTarget->GetRootComponent());
		break;
	}
	}

	return false;
}

bool UChrisSoundCueNotify::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	OnExecute_Implementation(MyTarget, Parameters);

	return false;
}
