// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyWidget.generated.h"

/**
 * 
 */
UCLASS()
class ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()


private:
	UPROPERTY(meta=(BindWidget))
	class UWidgetSwitcher* MainSwitcher;

	UPROPERTY(meta=(BindWidget))
	class UWidget* TeamSelectionRoot;

	UPROPERTY(meta=(BindWidget))
	class UButton* ReadyUpButton;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* RedTeamBox;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* BlueTeamBox;

};
