#include "CatGameInstance.h"
#include "CatQuestComponent.h"
#include "CatSaveSubsystem.h"
#include "CatCharacter.h"
#include "CatAppearanceComponent.h"
#include "EngineUtils.h"
#include "NPC.h"
#include "QuestData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

const FString UCatGameInstance::SaveSlotName = TEXT("CatSave");
const int32 UCatGameInstance::SaveUserIndex = 0;

void UCatGameInstance::Init()
{
    Super::Init();
    Inventory = NewObject<UCatInventoryComponent>(this, TEXT("PersistentInventory"));
    Inventory->RegisterComponent();
    // Inventory->OnInventoryChanged.AddDynamic(this, &UCatGameInstance::SaveGame);
}

void UCatGameInstance::SaveGame()
{
    if (bIsLoading) return;
    
    if (!ActiveSave)
    {
        ActiveSave = Cast<UCatSaveGame>(
            UGameplayStatics::CreateSaveGameObject(UCatSaveGame::StaticClass()));
    }

    if (!ActiveSave) return;

    PopulateSave(ActiveSave);
    UGameplayStatics::SaveGameToSlot(ActiveSave, SaveSlotName, SaveUserIndex);
}

bool UCatGameInstance::LoadGame()
{
    if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex))
    {
        NewGame();
        return false;
    }

    UCatSaveGame* Save = Cast<UCatSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex));

    if (!Save)
    {
        NewGame();
        return false;
    }

    bIsLoading = true;
    ActiveSave = Save;
    bPendingWorldRestore = true;

    ApplyNonWorldState(Save);

    UGameplayStatics::OpenLevel(this, FName("Main_Level"));

    return true;
}

void UCatGameInstance::NewGame()
{
    ActiveSave = nullptr;
    bPendingWorldRestore = false;
    LoadedSublevels.Empty();

    UGameplayStatics::OpenLevel(this, FName("Main_Level"));
}

void UCatGameInstance::BindPlayerSaveHooks(APawn* Player)
{
    bIsLoading = false;

    if (!Player) return;

    if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
        CachedPC = PC;

    UCatQuestComponent* QuestComp = Player->FindComponentByClass<UCatQuestComponent>();
    if (QuestComp)
    {
        // Clear existing bindings first to prevent double-binding
        QuestComp->OnQuestAccepted.RemoveDynamic(this, &UCatGameInstance::OnQuestAccepted);
        QuestComp->OnQuestCompleted.RemoveDynamic(this, &UCatGameInstance::OnQuestCompleted);
        QuestComp->OnQuestAccepted.AddDynamic(this, &UCatGameInstance::OnQuestAccepted);
        QuestComp->OnQuestCompleted.AddDynamic(this, &UCatGameInstance::OnQuestCompleted);

        if (ActiveSave)
        {
            QuestComp->RestoreFromSave(ActiveSave);

            if (!ActiveSave->ActiveMainQuestID.IsNone())
            {
                for (TActorIterator<ANPC> It(Player->GetWorld()); It; ++It)
                {
                    if (UQuestData* Quest = It->FindQuest(ActiveSave->ActiveMainQuestID))
                    {
                        QuestComp->RehydrateMainQuest(Quest);
                        break;
                    }
                }
            }
        }
    }

    if (ActiveSave)
    {
        Player->SetActorLocation(ActiveSave->PlayerLocation, false, nullptr, ETeleportType::TeleportPhysics);
        Player->SetActorRotation(ActiveSave->PlayerRotation);

        if (!ActiveSave->EquippedHatID.IsNone())
        {
            if (UCatAppearanceComponent* Appearance = Player->FindComponentByClass<UCatAppearanceComponent>())
            {
                UCatHatModifier* Hat = GetEquippedHat();
                if (Hat)
                    Appearance->SetHat(Hat);
            }
        }
    }
}

void UCatGameInstance::OnQuestAccepted(UQuestData* Quest)
{
    SaveGame();
}

void UCatGameInstance::OnQuestCompleted(UQuestData* Quest)
{
    SaveGame();
}

void UCatGameInstance::PopulateSave(UCatSaveGame* Save)
{
    Save->Inventory = Inventory->GetAllItems();

    APlayerController* PC = GetFirstLocalPlayerController();
    APawn* Pawn = PC ? PC->GetPawn() : nullptr;
    UCatQuestComponent* QuestComp = Pawn
        ? Pawn->FindComponentByClass<UCatQuestComponent>()
        : nullptr;

    if (QuestComp)
    {
        Save->CompletedQuestIDs = QuestComp->GetCompletedQuests();
        Save->ActiveSideQuestIDs = QuestComp->GetActiveQuests();

        UQuestData* MainQuest = QuestComp->GetActiveMainQuest();
        Save->ActiveMainQuestID = MainQuest ? MainQuest->QuestID : NAME_None;
    }

    if (Pawn)
    {
        Save->PlayerLocation = Pawn->GetActorLocation();
        Save->PlayerRotation = Pawn->GetActorRotation();
    }

    Save->LoadedSublevels = LoadedSublevels;

    if (ActiveSave)
    {
        Save->UnlockedHatIDs = ActiveSave->UnlockedHatIDs;
        Save->EquippedHatID = ActiveSave->EquippedHatID;
    }

    if (UWorld* World = GetWorld())
    {
        if (UCatSaveSubsystem* SaveSub = World->GetSubsystem<UCatSaveSubsystem>())
        {
            Save->WorldData = FWorldSaveData{};
            SaveSub->CollectWorldState(Save);
        }
    }
}

void UCatGameInstance::ApplyPendingWorldRestore()
{
    if (!bPendingWorldRestore || !ActiveSave) return;

    GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, TEXT("ApplyPendingWorldRestore fired"));
    
    if (UWorld* World = GetWorld())
    {
        if (UCatSaveSubsystem* SaveSub = World->GetSubsystem<UCatSaveSubsystem>())
            SaveSub->ApplyWorldState(ActiveSave);
    }

    bPendingWorldRestore = false;
}

void UCatGameInstance::ApplyNonWorldState(UCatSaveGame* Save)
{
    if (!Inventory)
    {
        Inventory = NewObject<UCatInventoryComponent>(this, TEXT("PersistentInventory"));
        Inventory->RegisterComponent();
        Inventory->OnInventoryChanged.AddDynamic(this, &UCatGameInstance::SaveGame);
    }

    Inventory->Clear();

    for (const TPair<FName, int32>& Pair : Save->Inventory)
        Inventory->AddItem(Pair.Key, Pair.Value);
}

void UCatGameInstance::NotifySubLevelLoaded(FName LevelName)
{
    LoadedSublevels.AddUnique(LevelName);

    if (!bPendingWorldRestore || !ActiveSave)
    {
        SaveGame();
        OnWorldRestoreComplete.Broadcast();

        if (CachedPC.IsValid())
        {
            if (ACharacter* Char = Cast<ACharacter>(CachedPC->GetPawn()))
                Char->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
            CachedPC->EnableInput(CachedPC.Get());
            CachedPC = nullptr;
        }

        return;
    }

    bool bAllLoaded = true;
    for (FName Expected : ActiveSave->LoadedSublevels)
    {
        if (!LoadedSublevels.Contains(Expected))
        {
            bAllLoaded = false;
            break;
        }
    }

    if (bAllLoaded)
    {
        ApplyPendingWorldRestore();
        OnWorldRestoreComplete.Broadcast();

        if (CachedPC.IsValid())
        {
            if (ACharacter* Char = Cast<ACharacter>(CachedPC->GetPawn()))
                Char->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
            CachedPC->EnableInput(CachedPC.Get());
            CachedPC = nullptr;
        }
    }
}

void UCatGameInstance::NotifySubLevelUnloaded(FName LevelName)
{
    LoadedSublevels.Remove(LevelName);
}

void UCatGameInstance::UnlockHat(UCatHatModifier* Hat)
{
    if (!Hat || Hat->HatID.IsNone()) return;

    if (!ActiveSave)
        ActiveSave = Cast<UCatSaveGame>(UGameplayStatics::CreateSaveGameObject(UCatSaveGame::StaticClass()));

    if (!ActiveSave) return;

    ActiveSave->UnlockedHatIDs.AddUnique(Hat->HatID);
    SaveGame();
}

UCatHatModifier* UCatGameInstance::GetNextUnlockedHat(UCatHatModifier* CurrentHat) const
{
    if (!ActiveSave || ActiveSave->UnlockedHatIDs.Num() == 0)
        return nullptr;

    // Collect unlocked hats in registry order so cycling is deterministic
    TArray<UCatHatModifier*> Unlocked;
    for (UCatHatModifier* Hat : AllHats)
    {
        if (Hat && ActiveSave->UnlockedHatIDs.Contains(Hat->HatID))
            Unlocked.Add(Hat);
    }

    if (Unlocked.Num() == 0)
        return nullptr;

    // nullptr (no hat) -> first unlocked
    if (!CurrentHat)
        return Unlocked[0];

    // Find current in unlocked list
    int32 Index = Unlocked.IndexOfByKey(CurrentHat);

    // Last unlocked (or not found) -> back to nullptr (no hat)
    if (Index == INDEX_NONE || Index == Unlocked.Num() - 1)
        return nullptr;

    return Unlocked[Index + 1];
}

UCatHatModifier* UCatGameInstance::GetEquippedHat() const
{
    if (!ActiveSave || ActiveSave->EquippedHatID.IsNone())
        return nullptr;

    for (UCatHatModifier* Hat : AllHats)
    {
        if (Hat && Hat->HatID == ActiveSave->EquippedHatID)
            return Hat;
    }

    return nullptr;
}

void UCatGameInstance::SetEquippedHat(UCatHatModifier* Hat)
{
    if (!ActiveSave)
        ActiveSave = Cast<UCatSaveGame>(UGameplayStatics::CreateSaveGameObject(UCatSaveGame::StaticClass()));

    if (!ActiveSave) return;

    ActiveSave->EquippedHatID = Hat ? Hat->HatID : NAME_None;
    SaveGame();
}