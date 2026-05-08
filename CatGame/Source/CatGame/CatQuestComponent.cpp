#include "CatQuestComponent.h"
#include "CatGameInstance.h"
#include "CatInventoryComponent.h"
#include "QuestEffectSubsystem.h"
#include "CatSaveGame.h"
#include "GameFramework/Actor.h"

UCatInventoryComponent* UCatQuestComponent::GetInventory() const
{
    UCatGameInstance* GI = Cast<UCatGameInstance>(GetWorld()->GetGameInstance());
    return GI ? GI->GetInventory() : nullptr;
}

bool UCatQuestComponent::AcceptQuest(UQuestData* Quest)
{
    if (!Quest || Quest->QuestID.IsNone())
        return false;

    if (IsQuestActive(Quest->QuestID) || IsQuestCompleted(Quest->QuestID))
        return false;

    KnownQuests.FindOrAdd(Quest->QuestID) = Quest;

    if (Quest->QuestType == EQuestType::Main)
    {
        // Evict the current main quest back to available
        // Can swap back whenever, quest isnt completed yet
        ActiveMainQuest = Quest;
    }
    else
    {
        ActiveSideQuestIDs.Add(Quest->QuestID);
    }

    OnQuestAccepted.Broadcast(Quest);
    return true;
}

bool UCatQuestComponent::CanCompleteQuest(UQuestData* Quest) const
{
    if (!Quest || !IsQuestActive(Quest->QuestID))
        return false;

    if (!Quest->RequiredItem)
        return true;

    UCatInventoryComponent* Inventory = GetInventory();
    if (!Inventory)
        return false;

    return Inventory->HasItem(Quest->RequiredItem->ItemID, Quest->RequiredQty);
}

bool UCatQuestComponent::CompleteQuest(UQuestData* Quest)
{
    if (!CanCompleteQuest(Quest))
        return false;

    UCatInventoryComponent* Inventory = GetInventory();

    if (Quest->RequiredItem)
        Inventory->RemoveItem(Quest->RequiredItem->ItemID, Quest->RequiredQty);

    if (Quest->RewardItem && Quest->RewardQty > 0)
        Inventory->AddItem(Quest->RewardItem->ItemID, Quest->RewardQty);

    if (Quest->RewardHat)
    {
        if (UCatGameInstance* GI = Cast<UCatGameInstance>(GetWorld()->GetGameInstance()))
            GI->UnlockHat(Quest->RewardHat);
    }

    if (Quest->QuestType == EQuestType::Main)
        ActiveMainQuest = nullptr;
    else
        ActiveSideQuestIDs.Remove(Quest->QuestID);

    CompletedQuestIDs.Add(Quest->QuestID);
    OnQuestCompleted.Broadcast(Quest);
    return true;
}

bool UCatQuestComponent::IsQuestActive(FName QuestID) const
{
    if (ActiveMainQuest && ActiveMainQuest->QuestID == QuestID)
        return true;

    return ActiveSideQuestIDs.Contains(QuestID);
}

bool UCatQuestComponent::IsQuestCompleted(FName QuestID) const
{
    return CompletedQuestIDs.Contains(QuestID);
}

UQuestData* UCatQuestComponent::GetActiveMainQuest() const
{
    return ActiveMainQuest;
}


void UCatQuestComponent::RestoreFromSave(const UCatSaveGame* Save)
{
    if (!Save) return;

    ActiveMainQuest = nullptr;
    ActiveSideQuestIDs.Empty();
    CompletedQuestIDs = Save->CompletedQuestIDs;
    ActiveSideQuestIDs = Save->ActiveSideQuestIDs;
}

void UCatQuestComponent::RehydrateMainQuest(UQuestData* Quest)
{
    if (Quest && !ActiveMainQuest)
        ActiveMainQuest = Quest;
}