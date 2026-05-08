#include "Door.h"
#include "CatGameInstance.h"
#include "CatInventoryComponent.h"
#include "CatSaveSubsystem.h"
#include "ItemData.h"

ADoor::ADoor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADoor::BeginPlay()
{
	Super::BeginPlay();

	bIsLocked = (LockItem != nullptr);

	if (!SaveID.IsNone())
	{
		if (UCatSaveSubsystem* SaveSub = GetWorld()->GetSubsystem<UCatSaveSubsystem>())
			SaveSub->RegisterSaveable(this);
	}
}

void ADoor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (!SaveID.IsNone())
	{
		if (UCatSaveSubsystem* SaveSub = GetWorld()->GetSubsystem<UCatSaveSubsystem>())
			SaveSub->UnregisterSaveable(this);
	}

	Super::EndPlay(EndPlayReason);
}

void ADoor::OnInteract_Implementation(AActor* InteractInstigator)
{
	if (LockItem)
	{
		if (bIsLocked)
		{
			UCatGameInstance* GI = Cast<UCatGameInstance>(GetGameInstance());
			if (!GI) return;

			UCatInventoryComponent* Inventory = GI->GetInventory();
			if (!Inventory || !Inventory->HasItem(LockItem->ItemID)) return;

			Inventory->RemoveItem(LockItem->ItemID);
			bIsLocked = false;
			
			GI->SaveGame();
			
			OnDoorUnlocked();
		}
		// If LockItem is set but door is already unlocked, fall through to toggle
	}

	if (!bIsLocked)
	{
		bIsOpen = !bIsOpen;
		if (bIsOpen) OnDoorOpened();
		else OnDoorClosed();
	}
}

void ADoor::SaveState(UCatSaveGame* SaveGame)
{
	if (!SaveGame || SaveID.IsNone()) return;

	FDoorSaveData Data;
	Data.bIsLocked = bIsLocked;
	Data.bIsOpen = bIsOpen;
	SaveGame->WorldData.Doors.Add(SaveID, Data);
}

void ADoor::RestoreState(const UCatSaveGame* SaveGame)
{
	if (!SaveGame || SaveID.IsNone()) return;

	const FDoorSaveData* Data = SaveGame->WorldData.Doors.Find(SaveID);
	if (!Data) return;

	bIsLocked = Data->bIsLocked;
	bIsOpen = Data->bIsOpen;
	
	if (bIsOpen) OnDoorOpened();
	else OnDoorClosed();

	if (!bIsLocked && LockItem) OnDoorUnlocked();
}