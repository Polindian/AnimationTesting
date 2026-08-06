// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VirtualKeyboardWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnKeyboardCommitted, const FText&);

/**
 * On-screen keyboard overlay for controller text entry (desktop UE has no built-in one).
 * Keys are spawned procedurally as MenuButtonWidgets. Shortcuts: B = cancel,
 * X = delete, Y = space. Added to the viewport on top, same pattern as the overlays.
 */
UCLASS()
class ANIMATIONTESTING_API UVirtualKeyboardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Resets the buffer to InitialText and takes focus. Safe on a reused instance.
	void OpenKeyboard(const FText& InitialText);

	FOnKeyboardCommitted OnCommitted;     // DONE — carries the final text
	FSimpleMulticastDelegate OnCancelled; // B — text discarded

protected:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PreviewText;

	// Empty VerticalBox the key rows get spawned into
	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* KeyRowsBox;

	// Bottom row lives in the Designer so it can be laid out nicely
	UPROPERTY(meta = (BindWidget))
	class UMenuButtonWidget* SpaceButton;

	UPROPERTY(meta = (BindWidget))
	class UMenuButtonWidget* DeleteButton;

	UPROPERTY(meta = (BindWidget))
	class UMenuButtonWidget* DoneButton;

	// Your WBP_MenuButton — spawned once per key
	UPROPERTY(EditDefaultsOnly, Category = "Keyboard")
	TSubclassOf<UMenuButtonWidget> KeyButtonClass;

	UPROPERTY(EditDefaultsOnly, Category = "Keyboard")
	int32 MaxLength = 20;

	UPROPERTY(EditDefaultsOnly, Category = "Keyboard")
	float KeySize = 64.f;

private:
	FString Buffer;

	// Key that receives default focus when the keyboard opens ('G' — center of the grid)
	UPROPERTY()
	class UMenuButtonWidget* HomeKey;

	UFUNCTION() void HandleKeyClicked(const FText& Label);
	UFUNCTION() void HandleSpace();
	UFUNCTION() void HandleDelete();
	UFUNCTION() void HandleDone();

	void CancelKeyboard();
	void RefreshPreview(bool bPlaySound = true);
};