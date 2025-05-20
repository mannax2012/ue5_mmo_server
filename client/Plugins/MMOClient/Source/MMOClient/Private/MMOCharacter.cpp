#include "MMOCharacter.h"
#include "MMOPlayerMovementComponent.h"
#include "Components/CapsuleComponent.h"

AMMOCharacter::AMMOCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UMMOPlayerMovementComponent>(ACharacter::CharacterMovementComponentName))
{
    MMOPlayerMovementComponent = Cast<UMMOPlayerMovementComponent>(GetCharacterMovement());
    if (MMOPlayerMovementComponent)
        MMOPlayerMovementComponent->UpdatedComponent = GetCapsuleComponent();
}
