// Christopher Naglik All Rights Reserved


#include "AI/BTTask_SendInputToAbilitySystem.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

EBTNodeResult::Type UBTTask_SendInputToAbilitySystem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* OwnerAIC = OwnerComp.GetAIOwner();
	if (OwnerAIC)
	{
		UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerAIC->GetPawn());
		if (OwnerASC)
		{
			OwnerASC->ReleaseInputID((int32)InputID);
			OwnerASC->PressInputID((int32)InputID);
			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}


