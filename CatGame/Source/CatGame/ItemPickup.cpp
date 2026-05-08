#include "ItemPickup.h"
#include "CatGameInstance.h"
#include "CatSaveSubsystem.h"
#include "CatSaveGame.h"
#include "ItemData.h"

void AItemPickup::BeginPlay()
{
	Super::BeginPlay();

	if (!SaveID.IsNone())
	{
		if (UCatSaveSubsystem* SaveSub = GetWorld()->GetSubsystem<UCatSaveSubsystem>())
			SaveSub->RegisterSaveable(this);
	}
}

void AItemPickup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (!SaveID.IsNone())
	{
		if (UCatSaveSubsystem* SaveSub = GetWorld()->GetSubsystem<UCatSaveSubsystem>())
			SaveSub->UnregisterSaveable(this);
	}

	Super::EndPlay(EndPlayReason);
}

void AItemPickup::FinishPickup()
{
	if (!SaveID.IsNone())
	{
		if (UCatSaveSubsystem* SaveSub = GetWorld()->GetSubsystem<UCatSaveSubsystem>())
			SaveSub->UnregisterSaveable(static_cast<ISaveableActor*>(this));
	}

	if (CollectableItem)
	{
		UCatGameInstance* GI = Cast<UCatGameInstance>(GetGameInstance());
		if (GI)
		{
			GI->GetInventory()->AddItem(CollectableItem->ItemID, 1);
			GI->SaveGame();
		}
	}

	Destroy();
}

void AItemPickup::SaveState(UCatSaveGame* SaveGame)
{
	if (!SaveGame || SaveID.IsNone()) return;
	
	FItemPickupSaveData Data;
	Data.bPickedUp = false;
	SaveGame->WorldData.ItemPickups.Add(SaveID, Data);
}

void AItemPickup::RestoreState(const UCatSaveGame* SaveGame)
{
	if (!SaveGame || SaveID.IsNone()) return;
	
	if (!SaveGame->WorldData.ItemPickups.Contains(SaveID))
		Destroy();
}