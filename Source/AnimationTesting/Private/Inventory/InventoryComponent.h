// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class UAbilitySystemComponent;
class UPA_ShopItem;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	void TryPurchase(const UPA_ShopItem* ItemToPurchase);
	float GetSoul() const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	UPROPERTY()
	UAbilitySystemComponent* OwnerAbilitySystemComponent;
	
// SERVER 
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Purchase(const UPA_ShopItem* ItemToPurchase);
};
