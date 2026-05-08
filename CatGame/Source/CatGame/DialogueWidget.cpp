#include "DialogueWidget.h"
#include "NPC.h"
#include "QuestEffectSubsystem.h"

void UDialogueWidget::InitDialogue(const FText& InNPCName, const TArray<FText>& InFirstTimeLines, const FText& InBarkLine, const TArray<FDialogueOption>& InOptions, bool bIsFirstTime, AActor* InNPC)
{
    Options = InOptions;
    NPCActor = InNPC;
    FirstTimeLines = InFirstTimeLines;
    CurrentLineIndex = 0;
    PendingQuest = nullptr;

    OnNPCNameChanged(InNPCName);

    if (bIsFirstTime && FirstTimeLines.Num() > 0)
    {
        DialogueMode = EDialogueMode::FirstTime;
        OnLineChanged(FirstTimeLines[0]);
        OnShowContinue();
    }
    else
    {
        DialogueMode = EDialogueMode::Options;
        OnLineChanged(InBarkLine);
        OnShowOptions(Options);
    }
}

void UDialogueWidget::AdvanceLine()
{
    switch (DialogueMode)
    {
    case EDialogueMode::QuestComplete:
        CurrentLineIndex++;
        if (CurrentLineIndex < QuestCompleteLines.Num())
        {
            OnLineChanged(QuestCompleteLines[CurrentLineIndex]);
            OnShowContinue();
        }
        else
        {
            DialogueMode = EDialogueMode::Options;
            CloseDialogue();
        }
        break;

    case EDialogueMode::QuestOffer:
        CurrentLineIndex++;
        if (CurrentLineIndex < QuestOfferLines.Num())
        {
            OnLineChanged(QuestOfferLines[CurrentLineIndex]);
            bool bIsLast = CurrentLineIndex == QuestOfferLines.Num() - 1;
            bIsLast ? OnShowAccept() : OnShowContinue();
        }
        break;

    case EDialogueMode::QuestNotReady:
        DialogueMode = EDialogueMode::Options;
        OnShowOptions(Options);
        break;

    case EDialogueMode::FirstTime:
        CurrentLineIndex++;
        if (CurrentLineIndex < FirstTimeLines.Num())
        {
            OnLineChanged(FirstTimeLines[CurrentLineIndex]);
        }
        else
        {
            DialogueMode = EDialogueMode::Options;
            OnShowOptions(Options);
        }
        break;

    default:
        break;
    }
}

void UDialogueWidget::SelectOption(FName OptionID)
{
    ANPC* NPC = Cast<ANPC>(NPCActor);

    if (OptionID.ToString().StartsWith("Quest_"))
    {
        FName QuestID = FName(*OptionID.ToString().Right(OptionID.ToString().Len() - 6));
        if (NPC)
        {
            UQuestData* Quest = NPC->FindQuest(QuestID);
            if (Quest)
            {
                PendingQuest = Quest;
                DialogueMode = EDialogueMode::QuestOffer;
                QuestOfferLines = Quest->OfferLines;
                QuestOfferLines.Add(Quest->GetSummaryText());
                CurrentLineIndex = 0;
                OnLineChanged(QuestOfferLines[0]);
                QuestOfferLines.Num() == 1 ? OnShowAccept() : OnShowContinue();
                return;
            }
            NPC->HandleQuestAccept(GetOwningPlayerPawn(), QuestID);
        }
        CloseDialogue();
        return;
    }

    if (OptionID.ToString().StartsWith("QuestNotReady_"))
    {
        FName QuestID = FName(*OptionID.ToString().Right(OptionID.ToString().Len() - 14));
        if (NPC)
        {
            UQuestData* Quest = NPC->FindQuest(QuestID);
            if (Quest)
            {
                PendingQuest = Quest;
                DialogueMode = EDialogueMode::QuestNotReady;
                QuestOfferLines.Empty();
                QuestOfferLines.Add(Quest->GetNotReadyText());
                CurrentLineIndex = 0;
                OnLineChanged(QuestOfferLines[0]);
                OnShowContinue();
                return;
            }
        }
        CloseDialogue();
        return;
    }

    if (OptionID.ToString().StartsWith("TurnIn_"))
    {
        FName QuestID = FName(*OptionID.ToString().Right(OptionID.ToString().Len() - 7));
        if (NPC)
        {
            UQuestData* Quest = NPC->FindQuest(QuestID);
            if (Quest)
            {
                // Stash effects to fire after dialogue closes
                PendingEffects = Quest->CompletionEffects;
            }

            NPC->HandleQuestTurnIn(GetOwningPlayerPawn(), QuestID);

            if (Quest && Quest->CompletionLines.Num() > 0)
            {
                DialogueMode = EDialogueMode::QuestComplete;
                QuestCompleteLines = Quest->CompletionLines;
                CurrentLineIndex = 0;
                OnLineChanged(QuestCompleteLines[0]);
                OnShowContinue();
                return;
            }
        }
        CloseDialogue();
        return;
    }

    if (OptionID == "Leave")
    {
        CloseDialogue();
        return;
    }
}

void UDialogueWidget::AcceptCurrentQuest()
{
    if (!PendingQuest) return;

    ANPC* NPC = Cast<ANPC>(NPCActor);
    if (NPC)
        NPC->HandleQuestAccept(GetOwningPlayerPawn(), PendingQuest->QuestID);

    CloseDialogue();
}

void UDialogueWidget::CloseDialogue()
{
    if (ANPC* NPC = Cast<ANPC>(NPCActor))
        NPC->OnDialogueFinished();

    // Fire any queued quest effects now that dialogue is closing
    if (PendingEffects.Num() > 0)
    {
        if (UQuestEffectSubsystem* FX = GetWorld()->GetSubsystem<UQuestEffectSubsystem>())
        {
            for (const FName& Tag : PendingEffects)
                FX->BroadcastEffect(Tag);
        }
        PendingEffects.Empty();
    }

    OnDialogueClosed();
    RemoveFromParent();

    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->SetShowMouseCursor(false);
    }
}