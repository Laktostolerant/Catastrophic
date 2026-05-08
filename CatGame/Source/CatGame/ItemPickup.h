#pragma once

#include "CoreMinimal.h"
#include "InteractableBase.h"
#include "SaveableActor.h"
#include "ItemPickup.generated.h"

class UItemData;

UCLASS()
class CATGAME_API AItemPickup : public AInteractableBase, public ISaveableActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable")
	TObjectPtr<UItemData> CollectableItem;

	UFUNCTION(BlueprintCallable, Category = "Interactable")
	void FinishPickup();

	virtual void SaveState(UCatSaveGame* SaveGame) override;
	virtual void RestoreState(const UCatSaveGame* SaveGame) override;
	virtual FName GetSaveID() const override { return SaveID; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};