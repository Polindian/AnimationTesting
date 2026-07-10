// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SessionEntryWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionEntrySelected, const FString& /*SelectedSessionIdString*/)

/**
 * 
 */
UCLASS()
class USessionEntryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	
	FOnSessionEntrySelected OnSessionEntrySelected;

	FORCEINLINE FString GetCachedSessionIdString() const { return CachedSessionIdString; }

	void InitializeEntry(const FString& Name, const FString& SessionIdStr);

private:
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* SessionNameText;

	UPROPERTY(meta = (BindWidget))
	class UButton* SessionButton;

	FString CachedSessionIdString;

	UFUNCTION()
	void SessionEntrySelected();
};
