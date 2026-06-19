// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ChrisNetStatics.generated.h"

/**
 * 
 */
UCLASS()
class UChrisNetStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	static uint8 GetPlayerCountPerTeam();
};
