#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "ForEachWithDelay.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FForEachWithDelayPin, int32, ArrayIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FForEachCompletedPin);

UCLASS()
class CATGAME_API UForEachWithDelay : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FForEachWithDelayPin LoopBody;

    UPROPERTY(BlueprintAssignable)
    FForEachCompletedPin Completed;

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "Utilities|Flow Control")
    static UForEachWithDelay* ForEachWithDelay(
        UObject* WorldContextObject,
        int32 Count,
        float DelayDuration,
        bool WaitForLoopBody
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UObject> WorldContextObject;

    int32 Count;
    float DelayDuration;
    bool WaitForLoopBody;
    int32 CurrentIndex;

    void ProcessNext();
    void OnDelayFinished();
};