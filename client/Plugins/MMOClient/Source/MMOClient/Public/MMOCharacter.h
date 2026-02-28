#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MMOCharacter.generated.h"

UCLASS(Blueprintable)
class MMOCLIENT_API AMMOCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    AMMOCharacter(const FObjectInitializer& ObjectInitializer);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MMO|Movement")
    class UMMOPlayerMovementComponent* MMOPlayerMovementComponent;

    // Unique entity ID for this character
    int32 EntityId = 0;
};
