// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MenuPlayerController.generated.h"

/**
 * Base controller for UI-only screens (menus, lobby).
 * Sets cursor visible, input mode to UI, and spawns a widget on BeginPlay.
 */
UCLASS()
class AMenuPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	virtual void OnRep_PlayerState() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Menu")
	TSubclassOf<UUserWidget> MenuWidgetClass;

	UPROPERTY()
	UUserWidget* MenuWidget;

	void SpawnWidget();
};
