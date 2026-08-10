// Christopher Naglik All Rights Reserved


#include "AnimNotify_ChrisSound.h"
#include "ChrisAudioSubsystem.h"
#include "ChrisSoundLibrary.h"
#include "Character/ChrisCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

void UAnimNotify_ChrisSound::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !SoundTag.IsValid()) { return; }

	const UWorld* World = MeshComp->GetWorld();
	if (!World) { return; }

	// Montage editor preview has no controller, so the local check would silence
	// everything — play unconditionally there or you can't place notifies by ear
	const bool bIsPreview = (World->WorldType == EWorldType::EditorPreview);

	AChrisCharacter* Character = Cast<AChrisCharacter>(MeshComp->GetOwner());

	if (!bIsPreview && bLocalOnly)
	{
		// Replicated montages run on every machine — without this, all five clients
		// hear the attacker's grunt
		if (!Character || !Character->IsLocallyControlledByPlayer())
		{
			return;
		}
	}

	UChrisSoundLibrary* Voice = (bUseCharacterVoice && Character)? Character->GetVoiceLibrary() : nullptr;

	if (UChrisAudioSubsystem* Audio = UChrisAudioSubsystem::Get(MeshComp))
	{
		Audio->Play2D(SoundTag, Voice);
	}
}

FString UAnimNotify_ChrisSound::GetNotifyName_Implementation() const
{
	// Shows the tag on the montage timeline instead of the class name
	return SoundTag.IsValid() ? SoundTag.ToString() : TEXT("Chris Sound");
}