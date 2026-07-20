// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TutorialBookWidget.generated.h"

class UImage;
class UTextBlock;
class UButton;
class UWidgetAnimation;

/**
 * Full-screen tutorial book overlay. Scales up from nothing over a background blur,
 * then shows 20 page textures as 10 double-page spreads with arrow/keyboard/gamepad paging.
 * Added to the viewport ON TOP of the menu — not a switcher page.
 */
UCLASS()
class ANIMATIONTESTING_API UTutorialBookWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Resets to spread 1 and plays the open animation. Safe to call on a reused instance.
	void OpenBook();

protected:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_LeftPage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_RightPage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_PageCounter;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_LeftArrow;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_RightArrow;

	// Visible close button — same action as Esc/Backspace/gamepad B
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UMenuButtonWidget> ReturnButton;

	// Panel holding pages/arrows/counter — hidden while the book is animating so
	// content only appears once the book is at full size
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> PageContent;

	// Played forward to open, in reverse to close. Also drives the background blur.
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Anim_OpenBook;

	// All 20 page textures in reading order: [0]=first left page, [1]=first right page, ...
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial Book")
	TArray<TObjectPtr<UTexture2D>> Pages;

private:
	// Which double-page spread is showing (0-based, so 0 = "1/10")
	int32 CurrentSpread = 0;

	// Lets one anim-finished handler serve both directions: forward = reveal pages, reverse = remove widget
	bool bClosing = false;

	UFUNCTION() void HandleLeftArrow();
	UFUNCTION() void HandleRightArrow();
	UFUNCTION() void HandleBookAnimFinished();
	UFUNCTION() void HandleReturnClicked();

	void ChangeSpread(int32 Delta);
	void RefreshSpread();
	void CloseBook();
	int32 NumSpreads() const { return Pages.Num() / 2; }
};