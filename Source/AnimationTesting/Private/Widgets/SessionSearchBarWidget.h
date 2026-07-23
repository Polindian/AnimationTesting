// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SessionSearchBarWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSearchTextChanged, const FText&);

/**
 * Session name bar with a navigable "highlighted" state separate from editing.
 * Keyboard: Enter on the highlighted bar starts typing, Enter again commits and
 * returns to highlighted. Controller: A on the highlighted bar requests the OSK.
 * Mouse: click the text directly, as normal.
 */
UCLASS()
class ANIMATIONTESTING_API USessionSearchBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Fired on every text change, from typing or the virtual keyboard
	FOnSearchTextChanged OnSearchTextChanged;

	// Controller pressed A while the bar was highlighted — owner opens the OSK
	FSimpleMulticastDelegate OnVirtualKeyboardRequested;

	FText GetText() const;

	// Puts the bar in the highlighted (navigable) state
	void FocusBar();

	// OSK commit path: sets text, fires the changed event, returns to highlighted
	void SetTextFromKeyboard(const FText& InText);

	// Nav rule target for the page's explicit navigation chain
	UWidget* GetBarButton() const;

	void ClearText();

protected:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// Glow covers BOTH states: bar highlighted and actively editing
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;

private:
	// Invisible-style button filling the bar — the navigation target
	UPROPERTY(meta = (BindWidget))
	class UButton* BarButton;

	// The actual text field, overlaid on top so mouse clicks reach it directly
	UPROPERTY(meta = (BindWidget))
	class UEditableText* InputText;

	UPROPERTY(meta = (BindWidget))
	class UImage* HighlightGlow;

	UFUNCTION()
	void HandleTextChanged(const FText& Text);

	UFUNCTION()
	void HandleTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
};