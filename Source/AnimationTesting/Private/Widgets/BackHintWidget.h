// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BackHintWidget.generated.h"

/**
 * Reusable input hint strip: [Backspace key icon] OR [gamepad B icon] + label.
 * Purely visual — never takes focus or input. Drop into any overlay/page.
 */
UCLASS()
class ANIMATIONTESTING_API UBackHintWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void SynchronizeProperties() override;

private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HintText;

	// Editable per-instance: "TO RETURN", "TO CANCEL", ...
	UPROPERTY(EditAnywhere, Category = "Back Hint")
	FText HintLabel = INVTEXT("TO RETURN");
};