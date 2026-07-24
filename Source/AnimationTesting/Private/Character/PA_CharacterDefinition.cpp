// Christopher Naglik All Rights Reserved


#include "Character/PA_CharacterDefinition.h"
#include "Character/ChrisCharacter.h"

FPrimaryAssetId UPA_CharacterDefinition::GetPrimaryAssetId() const
{
	// Combines the type string ("Character Definition") with this asset's name
	// to create a unique ID the Asset Manager uses to track and retrieve it

	return FPrimaryAssetId(GetCharacterDefinitionAssetType(), GetFName());
}

FPrimaryAssetType UPA_CharacterDefinition::GetCharacterDefinitionAssetType()
{
	return FPrimaryAssetType("Character Definition");
}

UTexture2D* UPA_CharacterDefinition::LoadIcon() const
{
	CharacterIcon.LoadSynchronous();
	if (CharacterIcon.IsValid())
	{
		return CharacterIcon.Get();
	}
	return nullptr;
}

TSubclassOf<AChrisCharacter> UPA_CharacterDefinition::LoadCharacterClass() const
{
	CharacterClass.LoadSynchronous();
	if (CharacterClass.IsValid())
	{
		return CharacterClass.Get();
	}

	return TSubclassOf<AChrisCharacter>();
}

TSubclassOf<UAnimInstance> UPA_CharacterDefinition::LoadDisplayAnimationBP() const
{
	DisplayAnimBP.LoadSynchronous();
	if (DisplayAnimBP.IsValid()) {
		return DisplayAnimBP.Get();
	}
	
	return TSubclassOf<UAnimInstance>();
}

USkeletalMesh* UPA_CharacterDefinition::LoadDisplayMesh() const
{
	TSubclassOf<AChrisCharacter> LoadedCharacterClass = LoadCharacterClass();
	if (!LoadedCharacterClass) return nullptr;

	ACharacter* Character = Cast<ACharacter>(LoadedCharacterClass.GetDefaultObject());
	if (!Character) return nullptr;

	return Character->GetMesh()->GetSkeletalMeshAsset();
}

UAnimSequenceBase* UPA_CharacterDefinition::LoadDisplayAnimation() const
{
	DisplayAnimation.LoadSynchronous();
	if (DisplayAnimation.IsValid())
	{
		return DisplayAnimation.Get();
	}
	return nullptr;
}

UTexture2D* UPA_CharacterDefinition::LoadToolTip() const
{
	ToolTipTexture.LoadSynchronous();
	if (ToolTipTexture.IsValid())
	{
		return ToolTipTexture.Get();
	}
	return nullptr;
}
