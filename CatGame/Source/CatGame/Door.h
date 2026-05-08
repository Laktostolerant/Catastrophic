#pragma once

#include "CoreMinimal.h"
#include "InteractableBase.h"
#include "SaveableActor.h"
#include "Door.generated.h"

class UItemData;

UCLASS()
class CATGAME_API ADoor : public AInteractableBase, public ISaveableActor
{
	GENERATED_BODY()

public:
	ADoor();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door")
	TObjectPtr<UItemData> LockItem;
	

	virtual void SaveState(UCatSaveGame* SaveGame) override;
	virtual void RestoreState(const UCatSaveGame* SaveGame) override;
	virtual FName GetSaveID() const override { return SaveID; }

protected:
	virtual void OnInteract_Implementation(AActor* InteractInstigator) override;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void OnDoorOpened();

	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void OnDoorClosed();

	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void OnDoorUnlocked();

	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void OnDoorLocked();

private:
	bool bIsOpen = false;
	bool bIsLocked = false;
};