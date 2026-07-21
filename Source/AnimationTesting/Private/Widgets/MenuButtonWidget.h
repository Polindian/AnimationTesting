// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuButtonWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMenuButtonClicked);

UCLASS()
class UMenuButtonWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeConstruct() override;
    virtual void SynchronizeProperties() override;

    // MainMenuWidget subscribes to this
    UPROPERTY(BlueprintAssignable, Category = "Menu Button")
    FOnMenuButtonClicked OnMenuButtonClicked;

    void SetButtonText(const FText& InText);

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

    UPROPERTY(EditDefaultsOnly, Category = "Menu Button")
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

    // Optional: for other buttons - defaults stay the same 
    UPROPERTY(EditAnywhere, Category = "Menu Button")
    FLinearColor HoverBackgroundColor = FLinearColor(0, 0, 0, 0);

    UPROPERTY(EditAnywhere, Category = "Menu Button")
    FLinearColor DefaultBackgroundColor = FLinearColor(0, 0, 0, 0);
};