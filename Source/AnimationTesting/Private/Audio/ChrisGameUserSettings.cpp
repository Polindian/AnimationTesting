// Christopher Naglik All Rights Reserved


#include "ChrisGameUserSettings.h"
#include "Engine/Engine.h"

UChrisGameUserSettings* UChrisGameUserSettings::GetChrisSettings()
{
	return GEngine ? Cast<UChrisGameUserSettings>(GEngine->GetGameUserSettings()) : nullptr;
}

void UChrisGameUserSettings::SetGraphicsQuality(int32 Level)
{
	GraphicsQuality = FMath::Clamp(Level, 1, 3);

	SetOverallScalabilityLevel(GraphicsQuality);

	// Applied here rather than on panel close, so the player sees the change while choosing
	ApplySettings(false);
}
