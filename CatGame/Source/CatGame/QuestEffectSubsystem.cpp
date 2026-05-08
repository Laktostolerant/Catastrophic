#include "QuestEffectSubsystem.h"
#include "Particles/ParticlePerfStatsManager.h"

void UQuestEffectSubsystem::RegisterListener(FName EffectTag, FQuestEffectDelegate Callback)
{
	Listeners.FindOrAdd(EffectTag).Add(MoveTemp(Callback));
}

void UQuestEffectSubsystem::BroadcastEffect(FName EffectTag)
{
	TArray<FQuestEffectDelegate>* Callbacks = Listeners.Find(EffectTag);
	if (!Callbacks) return;

	for (FQuestEffectDelegate& CB : *Callbacks)
		CB.ExecuteIfBound();
}