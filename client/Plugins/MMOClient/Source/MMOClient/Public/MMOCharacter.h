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

    /** Character Entity ID, readable from Blueprints */
    UPROPERTY(BlueprintReadOnly, Category = "MMO")
    int32 EntityId = 0;
    

    /** Character Level, readable from Blueprints */
    UPROPERTY(BlueprintReadOnly, Category = "MMO")
    int32 Level = 0;

    /** Character Class ID, readable from Blueprints */
    UPROPERTY(BlueprintReadOnly, Category = "MMO")
    int32 ClassId = 0;

    /** Character Name, accessible from Blueprints */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MMO")
    FString Name;
};
