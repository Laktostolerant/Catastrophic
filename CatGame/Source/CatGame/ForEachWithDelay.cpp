#include "ForEachWithDelay.h"
#include "Engine/World.h"
#include "TimerManager.h"

UForEachWithDelay* UForEachWithDelay::ForEachWithDelay(
    UObject* WorldContextObject,
    int32 Count,
    float DelayDuration,
    bool WaitForLoopBody)
{
    UForEachWithDelay* Node = NewObject<UForEachWithDelay>();
    Node->WorldContextObject = WorldContextObject;
    Node->Count = Count;
    Node->DelayDuration = DelayDuration;
    Node->WaitForLoopBody = WaitForLoopBody;
    Node->CurrentIndex = 0;
    return Node;
}

void UForEachWithDelay::Activate()
{
    if (Count <= 0)
    {
        Completed.Broadcast();
        SetReadyToDestroy();
        return;
    }
    ProcessNext();
}

void UForEachWithDelay::ProcessNext()
{
    if (CurrentIndex >= Count)
    {
        Completed.Broadcast();
        SetReadyToDestroy();
        return;
    }

    if (WaitForLoopBody)
    {
        // fire loop body first, then start delay after
        LoopBody.Broadcast(CurrentIndex);
    }

    FTimerHandle TimerHandle;
    FTimerDelegate Delegate;
    Delegate.BindUObject(this, &UForEachWithDelay::OnDelayFinished);
    WorldContextObject->GetWorld()->GetTimerManager().SetTimer(TimerHandle, Delegate, DelayDuration, false);

    if (!WaitForLoopBody)
    {
        // fire loop body immediately after starting the delay timer
        LoopBody.Broadcast(CurrentIndex);
    }
}

void UForEachWithDelay::OnDelayFinished()
{
    CurrentIndex++;
    ProcessNext();
}