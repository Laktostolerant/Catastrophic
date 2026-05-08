#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CatSaveSubsystem.generated.h"

class ISaveableActor;
class UCatSaveGame;

UCLASS()
class CATGAME_API UCatSaveSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void RegisterSaveable(ISaveableActor* Actor);
	
	void UnregisterSaveable(ISaveableActor* Actor);
	
	void CollectWorldState(UCatSaveGame* SaveGame);
	
	void ApplyWorldState(const UCatSaveGame* SaveGame);

private:
	TArray<AActor*> RegisteredSaveables;

	
	const UCatSaveGame* CachedSave = nullptr;
	bool bWorldRestoreApplied = false;
};