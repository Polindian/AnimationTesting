// Christopher Naglik All Rights Reserved


#include "Player/MenuPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "MenuPlayerController.h"

void AMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();
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
		if (MenuWidget)
		{
			MenuWidget->AddToViewport();
			MenuWidget->SetIsFocusable(true);

			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(MenuWidget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			SetShowMouseCursor(true);
		}
	}
}
