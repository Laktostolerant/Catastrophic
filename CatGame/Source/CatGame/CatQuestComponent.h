#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestData.h"
#include "CatQuestComponent.generated.h"

class UCatSaveGame;
class UCatInventoryComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CATGAME_API UCatQuestComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Quests")
    bool AcceptQuest(UQuestData* Quest);

    UFUNCTION(BlueprintPure, Category = "Quests")
    bool CanCompleteQuest(UQuestData* Quest) const;

    UFUNCTION(BlueprintCallable, Category = "Quests")
    bool CompleteQuest(UQuestData* Quest);

    UFUNCTION(BlueprintPure, Category = "Quests")
    bool IsQuestActive(FName QuestID) const;

    UFUNCTION(BlueprintPure, Category = "Quests")
    bool IsQuestCompleted(FName QuestID) const;

    void RehydrateMainQuest(UQuestData* Quest);
    
    UFUNCTION(BlueprintPure, Category = "Quests")
    UQuestData* GetActiveMainQuest() const;

    const TArray<FName>& GetActiveQuests() const { return ActiveSideQuestIDs; }
    const TArray<FName>& GetCompletedQuests() const { return CompletedQuestIDs; }

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestAccepted, UQuestData*, Quest);
    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FOnQuestAccepted OnQuestAccepted;

    // Broadcasts the completed quest so listeners know which one finished.
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, UQuestData*, Quest);
    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FOnQuestCompleted OnQuestCompleted;

    void RestoreFromSave(const UCatSaveGame* Save);
    
private:
    UPROPERTY()
    TObjectPtr<UQuestData> ActiveMainQuest;

    UPROPERTY()
    TArray<FName> ActiveSideQuestIDs;

    UPROPERTY()
    TArray<FName> CompletedQuestIDs;
    
    UPROPERTY()
    TMap<FName, TObjectPtr<UQuestData>> KnownQuests;

    UCatInventoryComponent* GetInventory() const;
};