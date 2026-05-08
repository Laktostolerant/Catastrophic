#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CatSaveGame.generated.h"

class UQuestData;
struct FItemPickupSaveData;

USTRUCT()
struct FDoorSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	bool bIsLocked = false;

	UPROPERTY()
	bool bIsOpen = false;
};

USTRUCT()
struct FQuestEffectSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHasBeenTriggered = false;
};

USTRUCT()
struct FNPCSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHasBeenTalkedTo = false;

	UPROPERTY()
	int32 CurrentQuestIndex = 0;
};

USTRUCT()
struct FItemPickupSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	bool bPickedUp = false;
};

USTRUCT()
struct FWorldSaveData
{
	GENERATED_BODY()

	// Keyed by SaveID
	UPROPERTY()
	TMap<FName, FDoorSaveData> Doors;

	UPROPERTY()
	TMap<FName, FQuestEffectSaveData> QuestEffects;

	UPROPERTY()
	TMap<FName, FNPCSaveData> NPCs;

	UPROPERTY()
	TMap<FName, FItemPickupSaveData> ItemPickups;
};

UCLASS()
class CATGAME_API UCatSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// ItemID -> Quantity
	UPROPERTY()
	TMap<FName, int32> Inventory;
	
	UPROPERTY()
	TArray<FName> CompletedQuestIDs;

	UPROPERTY()
	TArray<FName> ActiveSideQuestIDs;

	UPROPERTY()
	FName ActiveMainQuestID = NAME_None;

	// Player
	UPROPERTY()
	FVector PlayerLocation = FVector::ZeroVector;

	UPROPERTY()
	FRotator PlayerRotation = FRotator::ZeroRotator;
	
	UPROPERTY(BlueprintReadOnly)
	TArray<FName> LoadedSublevels;

	// Per-world actor state
	UPROPERTY()
	FWorldSaveData WorldData;
	
	UPROPERTY()
	TArray<FName> UnlockedHatIDs;

	UPROPERTY()
	FName EquippedHatID = NAME_None;
};