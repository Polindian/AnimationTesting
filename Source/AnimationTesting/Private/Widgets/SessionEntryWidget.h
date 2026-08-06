// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SessionEntryWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionEntrySelected, const FString& /*SelectedSessionIdString*/)

/**
 * One row in the session list. Focus drives the glow (keyboard, gamepad,
 * and mouse via hover->focus). Selection is a separate persistent visual —
 * the entry Join will use — and survives focus moving elsewhere.
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

	// Hover = focus, same as MenuButtonWidget — with ScrollWhenFocusChanges on the
	// scroll box, this is also what makes edge-hovered entries scroll into view
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// Focus drives the glow
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;

	// Focuses the inner button — for default focus and refresh refocus from MainMenuWidget
	void FocusEntry(bool bPlaySound = false);

	// Persistent "Join will use this one" indicator, independent of focus
	void SetSelectedVisual(bool bSelected);

	class UButton* GetSessionButton() const { return SessionButton; }

private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* SessionNameText;

	UPROPERTY(meta = (BindWidget))
	class UButton* SessionButton;

	UPROPERTY(meta = (BindWidget))
	class UImage* HoverGlow;

	// Text color of the entry Join will use — persists while focus moves elsewhere
	UPROPERTY(EditDefaultsOnly, Category = "Session Entry")
	FLinearColor SelectedTextColor = FLinearColor(1.f, 0.6f, 0.1f);   // orange

	UPROPERTY(EditDefaultsOnly, Category = "Session Entry")
	FLinearColor NormalTextColor = FLinearColor::White;

	FString CachedSessionIdString;

	UFUNCTION()
	void SessionEntrySelected();

	bool bSuppressFocusSound = false;
};