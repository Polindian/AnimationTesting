// Christopher Naglik All Rights Reserved


#include "Widgets/SessionEntryWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void USessionEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SessionButton->OnClicked.AddDynamic(this, &USessionEntryWidget::SessionEntrySelected);
}

void USessionEntryWidget::InitializeEntry(const FString& Name, const FString& SessionIdStr)
{
	SessionNameText->SetText(FText::FromString(Name));
	CachedSessionIdString = SessionIdStr;
}

void USessionEntryWidget::SessionEntrySelected()
{
	OnSessionEntrySelected.Broadcast(CachedSessionIdString);
}
