#include "NPC.h"
#include "CatAppearanceComponent.h"
#include "CatQuestComponent.h"
#include "CatSaveGame.h"
#include "CatSaveSubsystem.h"
#include "Components/CapsuleComponent.h"
#include "DialogueOption.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/PawnMovementComponent.h"

ANPC::ANPC()
{
    PrimaryActorTick.bCanEverTick = false;

    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
    Capsule->SetupAttachment(RootComponent);

    Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(Capsule);
}

void ANPC::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (UCatAppearanceComponent* Appearance = FindComponentByClass<UCatAppearanceComponent>())
        Appearance->ApplyToMesh(Mesh);

    ApplyPose();
}

void ANPC::BeginPlay()
{
    Super::BeginPlay();

    if (UCatAppearanceComponent* Appearance = FindComponentByClass<UCatAppearanceComponent>())
        Appearance->ApplyToMesh(Mesh);

    GetWorldTimerManager().SetTimerForNextTick(this, &ANPC::ApplyPose);

    if (!SaveID.IsNone())
    {
        if (UCatSaveSubsystem* SaveSub = GetWorld()->GetSubsystem<UCatSaveSubsystem>())
            SaveSub->RegisterSaveable(this);
    }
}

void ANPC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (!SaveID.IsNone())
    {
        if (UCatSaveSubsystem* SaveSub = GetWorld()->GetSubsystem<UCatSaveSubsystem>())
            SaveSub->UnregisterSaveable(this);
    }

    Super::EndPlay(EndPlayReason);
}

void ANPC::ApplyPose()
{
    TObjectPtr<UAnimSequence>* Found = PoseSequences.Find(CurrentPose);
    if (!Found || !*Found)
    {
        return;
    }

    Mesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
    Mesh->SetAnimation(*Found);
    Mesh->Play(true);
}

void ANPC::OnInteract_Implementation(AActor* InteractInstigator)
{
    if (!DialogueWidgetClass) return;
    if (ActiveDialogueWidget) return;

    APawn* InstigatorPawn = Cast<APawn>(InteractInstigator);
    if (!InstigatorPawn) return;

    APlayerController* PC = Cast<APlayerController>(InstigatorPawn->GetController());
    if (!PC) return;

    InstigatorPawn->GetMovementComponent()->StopMovementImmediately();
    InstigatorPawn->DisableInput(PC);

    TArray<FDialogueOption> Options = BuildDialogueOptions(InteractInstigator);
    FText BarkLine = GetBarkLine();
    bool bIsFirstTime = !bHasBeenTalkedTo;

    bHasBeenTalkedTo = true;

    ActiveDialogueWidget = CreateWidget<UDialogueWidget>(PC, DialogueWidgetClass);
    if (!ActiveDialogueWidget) return;

    ActiveDialogueWidget->AddToViewport();
    ActiveDialogueWidget->InitDialogue(
        NPCName,
        FirstTimeLines,
        BarkLine,
        Options,
        bIsFirstTime,
        this
    );

    FInputModeUIOnly InputMode;
    InputMode.SetWidgetToFocus(ActiveDialogueWidget->TakeWidget());
    PC->SetInputMode(InputMode);
    PC->SetShowMouseCursor(true);
}

FText ANPC::GetBarkLine() const
{
    if (BarkData && BarkData->BarkLines.Num() > 0)
    {
        int32 Index = FMath::RandRange(0, BarkData->BarkLines.Num() - 1);
        return BarkData->BarkLines[Index];
    }

    return NSLOCTEXT("NPC", "DefaultBark", "...");
}

TArray<FDialogueOption> ANPC::BuildDialogueOptions(AActor* InInstigator) const
{
    TArray<FDialogueOption> Options;

    UCatQuestComponent* QuestComp = InInstigator
        ? InInstigator->FindComponentByClass<UCatQuestComponent>()
        : nullptr;

    if (Quests.IsValidIndex(CurrentQuestIndex))
    {
        UQuestData* Quest = Quests[CurrentQuestIndex];
        if (Quest && !Quest->QuestID.IsNone())
        {
            FDialogueOption Option;

            if (QuestComp && QuestComp->IsQuestActive(Quest->QuestID))
            {
                if (QuestComp->CanCompleteQuest(Quest))
                {
                    Option.Label = FText::Format(
                        NSLOCTEXT("Dialogue", "TurnIn", "Here's the {0} you wanted."),
                        Quest->RequiredItem ? Quest->RequiredItem->DisplayName : Quest->Title);
                    Option.OptionID = FName(*FString::Printf(TEXT("TurnIn_%s"), *Quest->QuestID.ToString()));
                }
                else
                {
                    Option.Label = FText::Format(
                        NSLOCTEXT("Dialogue", "QuestProgress", "About {0}..."),
                        Quest->Title);
                    Option.OptionID = FName(*FString::Printf(TEXT("QuestNotReady_%s"), *Quest->QuestID.ToString()));
                }
            }
            else
            {
                Option.Label = FText::Format(
                    NSLOCTEXT("Dialogue", "AcceptQuest", "Quest: {0}"),
                    Quest->Title);
                Option.OptionID = FName(*FString::Printf(TEXT("Quest_%s"), *Quest->QuestID.ToString()));
            }

            Options.Add(Option);
        }
    }

    FDialogueOption LeaveOption;
    LeaveOption.Label = NSLOCTEXT("Dialogue", "Leave", "Leave");
    LeaveOption.OptionID = "Leave";
    Options.Add(LeaveOption);

    return Options;
}

void ANPC::OnDialogueFinished()
{
    ResetCooldown();
    
    if (ActiveDialogueWidget)
    {
        APlayerController* PC = ActiveDialogueWidget->GetOwningPlayer();
        if (PC && PC->GetPawn())
            PC->GetPawn()->EnableInput(PC);
    }

    ActiveDialogueWidget = nullptr;
}

void ANPC::HandleQuestAccept(AActor* InInstigator, FName QuestID)
{
    if (!InInstigator) return;
    UCatQuestComponent* QuestComp = InInstigator->FindComponentByClass<UCatQuestComponent>();
    if (!QuestComp) return;
    if (UQuestData* Quest = FindQuest(QuestID))
        QuestComp->AcceptQuest(Quest);
}

void ANPC::HandleQuestTurnIn(AActor* InInstigator, FName QuestID)
{
    if (!InInstigator) return;
    UCatQuestComponent* QuestComp = InInstigator->FindComponentByClass<UCatQuestComponent>();
    if (!QuestComp) return;
    if (UQuestData* Quest = FindQuest(QuestID))
    {
        if (QuestComp->CompleteQuest(Quest))
            CurrentQuestIndex++;
    }
}

UQuestData* ANPC::FindQuest(FName QuestID) const
{
    for (UQuestData* Quest : Quests)
    {
        if (Quest && Quest->QuestID == QuestID)
            return Quest;
    }
    return nullptr;
}

void ANPC::SaveState(UCatSaveGame* SaveGame)
{
    if (!SaveGame || SaveID.IsNone()) return;

    FNPCSaveData Data;
    Data.bHasBeenTalkedTo = bHasBeenTalkedTo;
    Data.CurrentQuestIndex = CurrentQuestIndex;
    SaveGame->WorldData.NPCs.Add(SaveID, Data);
}

void ANPC::RestoreState(const UCatSaveGame* SaveGame)
{
    if (!SaveGame || SaveID.IsNone()) return;

    const FNPCSaveData* Data = SaveGame->WorldData.NPCs.Find(SaveID);
    if (!Data) return;

    bHasBeenTalkedTo = Data->bHasBeenTalkedTo;
    CurrentQuestIndex = Data->CurrentQuestIndex;
}