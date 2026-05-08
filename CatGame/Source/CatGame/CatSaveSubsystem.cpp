#include "CatSaveSubsystem.h"
#include "SaveableActor.h"
#include "CatSaveGame.h"

void UCatSaveSubsystem::RegisterSaveable(ISaveableActor* Actor)
{
	if (!Actor) return;
	AActor* AsActor = Cast<AActor>(Actor);
	if (!AsActor) return;
	RegisteredSaveables.AddUnique(AsActor);

	if (bWorldRestoreApplied && CachedSave)
		Actor->RestoreState(CachedSave);
}

void UCatSaveSubsystem::UnregisterSaveable(ISaveableActor* Actor)
{
	AActor* AsActor = Cast<AActor>(Actor);
	if (!AsActor) return;
	RegisteredSaveables.Remove(AsActor);
}

void UCatSaveSubsystem::CollectWorldState(UCatSaveGame* SaveGame)
{
	if (!SaveGame) return;

	for (AActor* Actor : RegisteredSaveables)
	{
		if (Actor)
		{
			if (ISaveableActor* Saveable = Cast<ISaveableActor>(Actor))
			{
				Saveable->SaveState(SaveGame);
			}
		}
	}
}

void UCatSaveSubsystem::ApplyWorldState(const UCatSaveGame* SaveGame)
{
	if (!SaveGame) return;

	CachedSave = SaveGame;
	bWorldRestoreApplied = true;

	for (AActor* Actor : RegisteredSaveables)
	{
		if (Actor)
		{
			if (ISaveableActor* Saveable = Cast<ISaveableActor>(Actor))
				Saveable->RestoreState(SaveGame);
		}
	}
}