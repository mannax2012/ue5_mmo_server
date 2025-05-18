#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MMOClientDelegates.generated.h"

UINTERFACE(BlueprintType)
class UMMOClientDelegates : public UInterface
{
    GENERATED_BODY()
};

class MMOCLIENT_API IMMOClientDelegates
{
    GENERATED_BODY()
public:
    // Called when login result is received
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MMOClient|Delegates")
    void OnLoginResult(bool bSuccess);

    // Called when character select result is received
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MMOClient|Delegates")
    void OnCharSelectSuccess(bool bSuccess);

    // Called when character create result is received
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MMOClient|Delegates")
    void OnCharCreateSuccess(bool bSuccess);

    // Called when character delete result is received
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MMOClient|Delegates")
    void OnCharDeleteSuccess(bool bSuccess);

    // Called when character list result is received
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MMOClient|Delegates")
    void OnCharListResult(const TArray<FCharListEntry>& CharList, int32 CharCount);

    // Called when client state changes
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MMOClient|Delegates")
    void OnClientStateChanged(EMMOClientState NewState);

    // Called when Auth/Game/Chat socket connects
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MMOClient|Delegates")
    void OnAuthConnected();
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MMOClient|Delegates")
    void OnGameConnected();
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MMOClient|Delegates")
    void OnChatConnected();

    // Called when Auth/Game/Chat socket disconnects
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MMOClient|Delegates")
    void OnAuthDisconnected();
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MMOClient|Delegates")
    void OnGameDisconnected();
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MMOClient|Delegates")
    void OnChatDisconnected();
};
