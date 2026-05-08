#include "CatHUDWidget.h"
#include "CatGameInstance.h"
#include "CatInventoryComponent.h"
#include "CatQuestComponent.h"
#include "Components/TextBlock.h"
#include "QuestData.h"
#include "Components/VerticalBox.h"
#include "Blueprint/UserWidget.h"

void UCatHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (UCatGameInstance* GI = Cast<UCatGameInstance>(GetGameInstance()))
        GI->GetInventory()->OnInventoryChanged.AddDynamic(this, &UCatHUDWidget::RefreshInventory);

    if (APlayerController* PC = GetOwningPlayer())
    {
        if (APawn* Pawn = PC->GetPawn())
        {
            if (UCatQuestComponent* QuestComp = Pawn->FindComponentByClass<UCatQuestComponent>())
            {
                QuestComp->OnQuestAccepted.AddDynamic(this, &UCatHUDWidget::OnQuestAccepted);
                QuestComp->OnQuestCompleted.AddDynamic(this, &UCatHUDWidget::OnQuestCompleted);
            }
        }
    }

    RefreshInventory();
    RefreshQuest();
}

void UCatHUDWidget::RefreshInventory()
{
    if (!ItemListBox) return;
    ItemListBox->ClearChildren();

    UCatGameInstance* GI = Cast<UCatGameInstance>(GetGameInstance());
    if (!GI) return;

    UCatInventoryComponent* Inventory = GI->GetInventory();
    if (!Inventory) return;

    for (const TPair<FName, int32>& Entry : Inventory->GetAllItems())
    {
        if (!ItemRowWidgetClass) continue;

        UUserWidget* Row = CreateWidget<UUserWidget>(GetOwningPlayer(), ItemRowWidgetClass);
        if (!Row) continue;

        if (UTextBlock* NameText = Cast<UTextBlock>(Row->GetWidgetFromName(TEXT("ItemNameText"))))
            NameText->SetText(FText::FromName(Entry.Key));

        if (UTextBlock* QtyText = Cast<UTextBlock>(Row->GetWidgetFromName(TEXT("ItemQtyText"))))
            QtyText->SetText(FText::AsNumber(Entry.Value));

        ItemListBox->AddChild(Row);
    }
}

void UCatHUDWidget::RefreshQuest()
{
    APlayerController* PC = GetOwningPlayer();
    APawn* Pawn = PC ? PC->GetPawn() : nullptr;
    UCatQuestComponent* QuestComp = Pawn ? Pawn->FindComponentByClass<UCatQuestComponent>() : nullptr;

    UQuestData* MainQuest = QuestComp ? QuestComp->GetActiveMainQuest() : nullptr;

    if (!MainQuest)
    {
        if (QuestTitleText) QuestTitleText->SetText(FText::GetEmpty());
        if (QuestDescriptionText) QuestDescriptionText->SetText(FText::GetEmpty());
        if (QuestPanel) QuestPanel->SetVisibility(ESlateVisibility::Collapsed);
        return;
    }

    if (QuestTitleText) QuestTitleText->SetText(MainQuest->Title);
    if (QuestDescriptionText) QuestDescriptionText->SetText(MainQuest->Description);
    if (QuestPanel) QuestPanel->SetVisibility(ESlateVisibility::Visible);
}

void UCatHUDWidget::OnQuestAccepted(UQuestData* Quest)
{
    // Only refresh the HUD if a main quest was accepted
    // We don't show side quests.
    if (Quest && Quest->QuestType == EQuestType::Main)
        RefreshQuest();
}

void UCatHUDWidget::OnQuestCompleted(UQuestData* Quest)
{
    if (Quest && Quest->QuestType == EQuestType::Main)
        RefreshQuest();
}