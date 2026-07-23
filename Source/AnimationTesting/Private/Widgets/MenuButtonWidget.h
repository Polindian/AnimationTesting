// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuButtonWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMenuButtonClicked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMenuButtonClickedWithLabel, const FText&, Label);

class UButton;

UCLASS()
class UMenuButtonWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeOnInitialized() override;
    virtual void SynchronizeProperties() override;

    // MainMenuWidget subscribes to this
    UPROPERTY(BlueprintAssignable, Category = "Menu Button")
    FOnMenuButtonClicked OnMenuButtonClicked;

    void SetButtonText(const FText& InText);

    UButton* GetMainButton() const { return MainButton; }

    UPROPERTY(BlueprintAssignable, Category = "Menu Button")
    FOnMenuButtonClickedWithLabel OnMenuButtonClickedWithLabel;

    void SetButtonSize(float InWidth, float InHeight);

private:
    UPROPERTY(meta = (BindWidget))
    class UButton* MainButton;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* ButtonText;

    UPROPERTY(meta = (BindWidget))
    class URetainerBox* ButtonRetainer;

    // Editable per-instance in the Designer details panel
    UPROPERTY(EditAnywhere, Category = "Menu Button")
    FText ButtonLabel;

    UPROPERTY(EditAnywhere, Category = "Menu Button")
    UMaterialInterface* HoverGradientMaterial;

    UFUNCTION()
    void HandleClicked();
    UFUNCTION()
    void HandleHovered();
    UFUNCTION()
    void HandleUnhovered();

    UPROPERTY(meta = (BindWidget))
    class USizeBox* RootSizeBox;  

    UPROPERTY(EditAnywhere, Category = "Menu Button")
    float ButtonWidth = 300.f;

    UPROPERTY(EditAnywhere, Category = "Menu Button")
    float ButtonHeight = 80.f;

    // Full font control per instance: family, typeface, size, outline, spacing
    UPROPERTY(EditAnywhere, Category = "Menu Button")
    FSlateFontInfo ButtonFont;


    /*********************************/
    /*     UE Navigation Focus       */
    /*********************************/

public:
    // Gives this button user focus — used for default page highlighting and mouse override
    void FocusButton();

protected:
    // Focus drives the highlight now — these fire when this widget (or its inner button) gains/loses focus, whether from keyboard, gamepad, or mouse hover
    virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
    virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;



};