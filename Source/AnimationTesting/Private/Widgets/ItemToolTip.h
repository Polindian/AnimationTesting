// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemToolTip.generated.h"

/**
 * 
 */
UCLASS()
class UItemToolTip : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetTooltipImage(UTexture2D* InTexture);
	void SetTooltipSize(FVector2D InSize);


private:
	UPROPERTY(meta = (BindWidget))
	class UImage* TooltipImage;
};
