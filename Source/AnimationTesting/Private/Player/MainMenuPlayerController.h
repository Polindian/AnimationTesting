// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Player/MenuPlayerController.h"
#include "MainMenuPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class AMainMenuPlayerController : public AMenuPlayerController
{
	GENERATED_BODY()
	
public:
	AMainMenuPlayerController();

	void StartMenuMusic();
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(Transient)
	class UAudioComponent* MenuMusic = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	float MusicFadeInTime = 2.f;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	float MusicFadeOutTime = 1.5f;
};
