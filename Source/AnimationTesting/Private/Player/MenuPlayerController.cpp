// Christopher Naglik All Rights Reserved


#include "Player/MenuPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "MenuPlayerController.h"

void AMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();
	SetInputMode(FInputModeUIOnly());
	SetShowMouseCursor(true);

	if (HasAuthority() && IsLocalPlayerController())
	{
		SpawnWidget();
	}
}

void AMenuPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	if (IsLocalPlayerController())
	{
		SpawnWidget();
	}
}

void AMenuPlayerController::SpawnWidget()
{
	if (MenuWidgetClass)
	{
		MenuWidget = CreateWidget<UUserWidget>(this, MenuWidgetClass);
		if(MenuWidget)
		{
			MenuWidget->AddToViewport();
		}
	}
}
