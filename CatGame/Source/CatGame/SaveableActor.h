#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SaveableActor.generated.h"

class UCatSaveGame;

UINTERFACE(MinimalAPI)
class USaveableActor : public UInterface
{
	GENERATED_BODY()
};

class CATGAME_API ISaveableActor
{
	GENERATED_BODY()

public:
	virtual void SaveState(UCatSaveGame* SaveGame) = 0;
	
	virtual void RestoreState(const UCatSaveGame* SaveGame) = 0;
	
	virtual FName GetSaveID() const = 0;
};