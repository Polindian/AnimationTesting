// Christopher Naglik All Rights Reserved


#include "ChrisSoundCueNotify.h"
#include "ChrisAudioSubsystem.h"
#include "ChrisSoundLibrary.h"
#include "Character/ChrisCharacter.h"
#include "GameFramework/Pawn.h"

bool UChrisSoundCueNotify::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget || !SoundTag.IsValid()) { return false; }

	AChrisCharacter* Character = Cast<AChrisCharacter>(MyTarget);

	if (bOwnerOnly2D)
	{
		// Cues fire on every client — without this, all five players hear the
		// victim's grunt. IsPlayerControlled excludes AI on the server.
		const APawn* Pawn = Cast<APawn>(MyTarget);
		if (!Pawn || !Pawn->IsLocallyControlled() || !Pawn->IsPlayerControlled())
		{
			return false;
		}
	}

	UChrisSoundLibrary* Voice = (bUseCharacterVoice && Character)
		? Character->GetVoiceLibrary()
		: nullptr;

	if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(MyTarget))
	{
		Audio->Play2D(SoundTag, Voice);
	}

	return false;
}