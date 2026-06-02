// Christopher Naglik All Rights Reserved


#include "Widgets/ItemToolTip.h"
#include "Components/Image.h"

void UItemToolTip::SetTooltipImage(UTexture2D* InTexture)
{
    if (TooltipImage && InTexture)
    {
        TooltipImage->SetBrushFromTexture(InTexture);
    }
}


void UItemToolTip::SetTooltipSize(FVector2D InSize)
{
    if (TooltipImage)
    {
        FSlateBrush Brush = TooltipImage->GetBrush();
        Brush.ImageSize = InSize;
        TooltipImage->SetBrush(Brush);
    }
}