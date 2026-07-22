// Christopher Naglik All Rights Reserved

#include "Widgets/BackHintWidget.h"
#include "Components/TextBlock.h"

// Runs in the editor too — label shows up live in the Designer
void UBackHintWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (HintText)
	{
		HintText->SetText(HintLabel);
	}
}