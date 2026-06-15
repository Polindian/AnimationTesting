// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "ChrisPlayerController.generated.h"


class UMatchCountdownWidget;
class URoundTimerWidget;
class UShopWidget;

/**
 * 
 */
UCLASS()
class AChrisPlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
public:
	// only called on the server
	void OnPossess(APawn* NewPawn) override;
	// only called on the client and listening server
	void AcknowledgePossession(APawn* NewPawn) override;

	/** Assigns Team Agent to given TeamID */
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

	/** Retrieve team identifier in form of FGenericTeamId */
	virtual FGenericTeamId GetGenericTeamId() const override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(Client, Reliable)
    void Client_OnFadeFromBlack(float Duration);

    UFUNCTION(Client, Reliable)
    void Client_OnSetShopCamera();

    UFUNCTION(Client, Reliable)
    void Client_OnSetArenaCamera();


	// ---- Client RPCs for match flow ---- 

     // Called when countdown begins. Hides gameplay UI, shows countdown number.
    UFUNCTION(Client, Reliable)
    void Client_OnCountdownStart(int32 Seconds);

    // Called every second during countdown. Updates the displayed number.
    // SecondsRemaining == 0 means show "FIGHT!"
    UFUNCTION(Client, Reliable)
    void Client_OnCountdownTick(int32 SecondsRemaining);

    // Called when the round starts. Enables input, shows gameplay UI + round timer.
    UFUNCTION(Client, Reliable)
    void Client_OnRoundStart(float Duration);

    // Called when the round ends. Disables input, hides round timer.
    UFUNCTION(Client, Reliable)
    void Client_OnRoundEnd();

    // Called to fade the camera to black over Duration seconds.
    UFUNCTION(Client, Reliable)
    void Client_OnFadeToBlack(float Duration);

    // Called at transition midpoint (screen is black).
    // Swaps to shop UI, then fades camera back in.
    UFUNCTION(Client, Reliable)
    void Client_OnShopPhaseStart(float InShopDuration, float FadeInDuration);

    // Called at transition midpoint back to arena.
    // Swaps to gameplay UI, then fades camera back in.
    UFUNCTION(Client, Reliable)
    void Client_OnReturnToArena(float FadeInDuration);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_VoteContinue();


    // Resetting round rotation for all players
    UFUNCTION(Client, Reliable)
    void Client_OnResetRotation(FRotator SpawnRotation);
private:

	void SpawnGameplayWidget();
	UPROPERTY()
	class AChrisPlayerCharacter* ChrisPlayerCharacter;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UGameplayWidget> GameplayWidgetClass;

	UPROPERTY()
	class UGameplayWidget* GameplayWidget;

	UPROPERTY(Replicated)
	FGenericTeamId TeamID;

    // Widget class references
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UMatchCountdownWidget> CountdownWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<URoundTimerWidget> RoundTimerWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UShopWidget> ShopWidgetClass;

    // Widget instance pointers
    UPROPERTY()
    UMatchCountdownWidget* CountdownWidget;

    UPROPERTY()
    URoundTimerWidget* RoundTimerWidget;

    UPROPERTY()
    UShopWidget* ShopWidget;

    // Camera references
    FRotator OriginalBoomRotation;
    FVector OriginalBoomSocketOffset;
    bool bOriginalUsePawnControlRotation = true;
    float OriginalArmLength = 0.f;

    FRotator InitialSpawnRotation;

    FTimerHandle ShopFadeTimerHandle;


public:
	virtual void SetupInputComponent() override;

 private:
	UFUNCTION()
	void ToggleGameplayMenu();

	UPROPERTY(EditDefaultsOnly, Category = "Input")
    class UInputMappingContext* UIInputMapping;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    class UInputAction* ToggleGameplayMenuAction;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<class UGameplayMenu> GameplayMenuClass;

    UPROPERTY()
    class UGameplayMenu* GameplayMenuWidget;

    bool bIsGameplayMenuOpen = false;

    bool bIsRoundActive = false;
};
