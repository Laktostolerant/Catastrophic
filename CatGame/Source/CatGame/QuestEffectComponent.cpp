#include "QuestEffectComponent.h"

#include "CatSaveGame.h"
#include "CatSaveSubsystem.h"

void UQuestEffectComponent::BeginPlay()
{
	Super::BeginPlay();

	if (QuestEffectTag.IsNone()) return;

	if (UCatSaveSubsystem* SaveSub = GetWorld()->GetSubsystem<UCatSaveSubsystem>())
		SaveSub->RegisterSaveable(this);

	if (UQuestEffectSubsystem* FX = GetWorld()->GetSubsystem<UQuestEffectSubsystem>())
	{
		FX->RegisterListener(QuestEffectTag, FQuestEffectDelegate::CreateLambda([this]()
		{
			bHasBeenTriggered = true;
			OnQuestEffectTriggered.Broadcast();
		}));
	}
}

void UQuestEffectComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (!QuestEffectTag.IsNone())
	{
		if (UCatSaveSubsystem* SaveSub = GetWorld()->GetSubsystem<UCatSaveSubsystem>())
			SaveSub->UnregisterSaveable(this);
	}

	Super::EndPlay(EndPlayReason);
}

void UQuestEffectComponent::SaveState(UCatSaveGame* SaveGame)
{
	if (!SaveGame || QuestEffectTag.IsNone()) return;

	FQuestEffectSaveData Data;
	Data.bHasBeenTriggered = bHasBeenTriggered;
	SaveGame->WorldData.QuestEffects.Add(QuestEffectTag, Data);
}

void UQuestEffectComponent::RestoreState(const UCatSaveGame* SaveGame)
{
	if (!SaveGame || QuestEffectTag.IsNone()) return;

	const FQuestEffectSaveData* Data = SaveGame->WorldData.QuestEffects.Find(QuestEffectTag);
	if (!Data) return;

	if (Data->bHasBeenTriggered && !bHasBeenTriggered)
	{
		bHasBeenTriggered = true;
		// Re-broadcast so the bridge/door/whatever responds correctly on load.
		OnQuestEffectTriggered.Broadcast();
	}
}

void UQuestEffectComponent::TriggerEffect()
{
	if (UQuestEffectSubsystem* FX = GetWorld()->GetSubsystem<UQuestEffectSubsystem>())
		FX->BroadcastEffect(QuestEffectTag);
}