// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QualityOptionWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnQualityOptionChosen, int32 /*Level*/);

/**
 * One row of a radio group — a box that shows a tick when selected.
 * The parent owns which one is ticked; this only reports being chosen.
 */
UCLASS()
class ANIMATIONTESTING_API UQualityOptionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnQualityOptionChosen OnQualityOptionChosen;

	// Shows or hides the tick. Never broadcasts — the parent drives this
	void SetChosen(bool bChosen);

	void FocusOption();

	// Scalability level this row represents: 0 Low, 1 Medium, 2 High
	int32 GetQualityLevel() const { return QualityLevel; }

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// Transparent button filling the row — the nav target, same as your leaderboard rows
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> OptionButton;

	// White square. Plain border brush, no texture needed
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> CheckBox;

	// White tick texture, tinted at runtime. Hidden until chosen
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> TickIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> HoverGlow;

	// Set per instance in the settings WBP: 0, 1 or 2
	UPROPERTY(EditAnywhere, Category = "Quality Option")
	int32 QualityLevel = 0;

	UPROPERTY(EditAnywhere, Category = "Quality Option")
	FLinearColor TickColor = FLinearColor(1.f, 0.65f, 0.15f, 1.f);

private:
	void HandleClicked();
};