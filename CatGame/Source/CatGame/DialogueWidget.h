#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogueOption.h"
#include "QuestData.h"
#include "DialogueWidget.generated.h"

UENUM()
enum class EDialogueMode : uint8
{
	Options,
	FirstTime,
	QuestOffer,
	QuestNotReady,
	QuestComplete
};

UCLASS()
class CATGAME_API UDialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitDialogue(const FText& InNPCName, const TArray<FText>& InFirstTimeLines, const FText& InBarkLine, const TArray<FDialogueOption>& InOptions, bool bIsFirstTime, AActor* InNPC);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void AdvanceLine();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SelectOption(FName OptionID);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void AcceptCurrentQuest();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	TArray<FDialogueOption> Options;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	bool bInFirstTimeMode = false;

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void OnLineChanged(const FText& Line);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void OnShowOptions(const TArray<FDialogueOption>& InOptions);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void OnShowContinue();

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void OnShowAccept();

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void OnDialogueClosed();

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void OnNPCNameChanged(const FText& Name);

private:
	UPROPERTY()
	TObjectPtr<AActor> NPCActor;

	UPROPERTY()
	TObjectPtr<UQuestData> PendingQuest;

	TArray<FText> FirstTimeLines;
	TArray<FText> QuestOfferLines;
	TArray<FText> QuestCompleteLines;
	int32 CurrentLineIndex = 0;
	
	EDialogueMode DialogueMode = EDialogueMode::Options;

	void CloseDialogue();
	
	TArray<FName> PendingEffects;
};
