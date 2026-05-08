#pragma once

#include "CoreMinimal.h"
#include "InteractableBase.h"
#include "NPCBarkData.h"
#include "DialogueOption.h"
#include "DialogueWidget.h"
#include "QuestData.h"
#include "SaveableActor.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimSequence.h"
#include "NPC.generated.h"

UENUM(BlueprintType)
enum class ENPCPose : uint8
{
    Idle    UMETA(DisplayName = "Idle"),
    Sit1    UMETA(DisplayName = "Sit 1"),
    Sit2    UMETA(DisplayName = "Sit 2"),
    Lay1    UMETA(DisplayName = "Lay 1"),
    Lay2    UMETA(DisplayName = "Lay 2"),
    Sleep1   UMETA(DisplayName = "Sleep 1"),
    Sleep2   UMETA(DisplayName = "Sleep 2"),
    Sleep3   UMETA(DisplayName = "Sleep 3"),
    Eat1     UMETA(DisplayName = "Eat 1"),
};

UCLASS()
class CATGAME_API ANPC : public AInteractableBase, public ISaveableActor
{
    GENERATED_BODY()

public:
    ANPC();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
    TObjectPtr<USkeletalMeshComponent> Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
    TObjectPtr<UCapsuleComponent> Capsule;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
    FText NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Pose")
    ENPCPose CurrentPose = ENPCPose::Idle;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Pose")
    TMap<ENPCPose, TObjectPtr<UAnimSequence>> PoseSequences;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TArray<FText> FirstTimeLines;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TObjectPtr<UNPCBarkData> BarkData;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TSubclassOf<UDialogueWidget> DialogueWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quests")
    TArray<TObjectPtr<UQuestData>> Quests;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void OnDialogueFinished();

    UFUNCTION(BlueprintCallable, Category = "Quests")
    void HandleQuestAccept(AActor* InInstigator, FName QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quests")
    void HandleQuestTurnIn(AActor* InInstigator, FName QuestID);

    UFUNCTION(BlueprintPure, Category = "Quests")
    UQuestData* FindQuest(FName QuestID) const;
    

    virtual void SaveState(UCatSaveGame* SaveGame) override;
    virtual void RestoreState(const UCatSaveGame* SaveGame) override;
    virtual FName GetSaveID() const override { return SaveID; }
    
protected:
    virtual void BeginPlay() override;
    void EndPlay(EEndPlayReason::Type EndPlayReason);
    virtual void OnInteract_Implementation(AActor* InteractInstigator) override;
    virtual void OnConstruction(const FTransform& Transform) override;

    FText GetBarkLine() const;
    TArray<FDialogueOption> BuildDialogueOptions(AActor* InInstigator) const;

private:
    UPROPERTY()
    TObjectPtr<UDialogueWidget> ActiveDialogueWidget;

    bool bHasBeenTalkedTo = false;
    int32 CurrentQuestIndex = 0;

    void ApplyPose();
};