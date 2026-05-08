#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CatHUDWidget.generated.h"

class UQuestData;
class UTextBlock;
class UVerticalBox;
class UCatGameInstance;
class UWidget;

UCLASS()
class CATGAME_API UCatHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void RefreshInventory();

	// Pulls the current main quest from the quest component and updates the display. Nullptr clears.
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void RefreshQuest();

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> ItemListBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> QuestPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> QuestTitleText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> QuestDescriptionText;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UUserWidget> ItemRowWidgetClass;

private:
	UFUNCTION()
	void OnQuestAccepted(UQuestData* Quest);

	UFUNCTION()
	void OnQuestCompleted(UQuestData* Quest);
};