// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "ChrisPlayerController.generated.h"

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
};
