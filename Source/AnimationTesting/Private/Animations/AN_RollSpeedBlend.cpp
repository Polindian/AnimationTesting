// Christopher Naglik All Rights Reserved


#include "Animations/AN_RollSpeedBlend.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/ChrisAbilitySystemStatics.h"

void UAN_RollSpeedBlend::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp || !MeshComp->GetOwner()) return;
    UE_LOG(LogTemp, Warning, TEXT("AN_RollSpeedBlend FIRED on %s"), *MeshComp->GetOwner()->GetName());

    ACharacter* OwnerCharacter = Cast<ACharacter>(MeshComp->GetOwner());
    if (!OwnerCharacter) return;

    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
    if (!MovementComp) return;

    // Remove the speed override tag so the AnimInstance can take back control
    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerCharacter);
    if (ASC)
    {
        ASC->RemoveLooseGameplayTag(UChrisAbilitySystemStatics::GetSpeedOverrideTag());
    }

    // Set speed to the blend target — the AnimInstance will now smoothly
    // interpolate from this toward the direction-based locomotion speed
    MovementComp->MaxWalkSpeed = BlendToSpeed;
}