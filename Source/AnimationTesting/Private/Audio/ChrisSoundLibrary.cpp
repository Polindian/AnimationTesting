// Christopher Naglik All Rights Reserved


#include "Audio/ChrisSoundLibrary.h"


const FChrisSoundDef* UChrisSoundLibrary::FindSound(const FGameplayTag& Tag) const
{
	// audio.ui.shop.unlock.skill -> audio.ui.shop.unlock -> audio.ui.shop -> ...
	// RequestDirectParent() returns an invalid tag at the root, ending the loop.
	for (FGameplayTag Current = Tag; Current.IsValid(); Current = Current.RequestDirectParent())
	{
		if (const FChrisSoundDef* Found = Sounds.Find(Current))
		{
			if (Found->Sound)
			{
				return Found;
			}
		}
	}
	return nullptr;
}

