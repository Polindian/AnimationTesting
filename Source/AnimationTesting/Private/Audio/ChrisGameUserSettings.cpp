// Christopher Naglik All Rights Reserved


#include "ChrisGameUserSettings.h"
#include "Engine/Engine.h"

UChrisGameUserSettings* UChrisGameUserSettings::GetChrisSettings()
{
	return GEngine ? Cast<UChrisGameUserSettings>(GEngine->GetGameUserSettings()) : nullptr;
}