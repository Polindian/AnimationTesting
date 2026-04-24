// Christopher Naglik All Rights Reserved


#include "Animations/AN_SendTargetGroup.h"
#include "AbilitySystemBlueprintLibrary.h"

void UAN_SendTargetGroup::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) 
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	if (TargetSocketNames.Num() <= 1) return;

	if (!MeshComp->GetOwner() || !UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner()))
	{
		return;
	}

	FGameplayEventData Data;

	for (int i = 1; i < TargetSocketNames.Num(); i++)
	{
		//heap allocation
		FGameplayAbilityTargetData_LocationInfo* LocationInfo = new FGameplayAbilityTargetData_LocationInfo();

		FVector StartLocation = MeshComp->GetSocketLocation(TargetSocketNames[i - 1]);
		FVector EndLocation = MeshComp->GetSocketLocation(TargetSocketNames[i]);

		UE_LOG(LogTemp, Warning, TEXT("Creating trace: %s ? %s (Start: %s, End: %s)"),
			*TargetSocketNames[i - 1].ToString(),
			*TargetSocketNames[i].ToString(),
			*StartLocation.ToString(),
			*EndLocation.ToString());

		LocationInfo->SourceLocation.LiteralTransform.SetLocation(StartLocation);
		LocationInfo->TargetLocation.LiteralTransform.SetLocation(EndLocation);

		Data.TargetData.Add(LocationInfo);
	}

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), EventTag, Data);
}