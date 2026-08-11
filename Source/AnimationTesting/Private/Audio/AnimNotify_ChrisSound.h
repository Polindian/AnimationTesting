// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "ChrisSoundLibrary.h"
#include "AnimNotify_ChrisSound.generated.h"


UCLASS(const, hidecategories = Object, collapsecategories, meta = (DisplayName = "Chris Sound"))
class ANIMATIONTESTING_API UAnimNotify_ChrisSound : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

	UPROPERTY(EditAnywhere, Category = "Chris Sound", meta = (Categories = "audio"))
	FGameplayTag SoundTag;

	UPROPERTY(EditAnywhere, Category = "Chris Sound")
	EChrisCueSoundMode Mode = EChrisCueSoundMode::OwnerOnly2D;

	/** Checks the character's voice library before the global one */
	UPROPERTY(EditAnywhere, Category = "Chris Sound")
	bool bUseCharacterVoice = true;
};