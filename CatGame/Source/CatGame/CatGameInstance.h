#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CatInventoryComponent.h"
#include "CatAppearance/CatHatModifier.h"
#include "CatSaveGame.h"
#include "QuestData.h"
#include "CatGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWorldRestoreComplete);

UCLASS()
class CATGAME_API UCatGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	UCatInventoryComponent* GetInventory() const { return Inventory; }

	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveGame();

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadGame();

	UFUNCTION(BlueprintCallable, Category = "Save")
	void NewGame();

	UFUNCTION(BlueprintPure, Category = "Save")
	UCatSaveGame* GetActiveSave() const { return ActiveSave; }

	UFUNCTION(BlueprintPure, Category = "Save")
	bool HasPendingWorldRestore() const { return bPendingWorldRestore; }

	void ApplyPendingWorldRestore();

	UFUNCTION(BlueprintCallable, Category = "Save")
	void NotifySubLevelLoaded(FName LevelName);

	UFUNCTION(BlueprintCallable, Category = "Save")
	void NotifySubLevelUnloaded(FName LevelName);

	UFUNCTION(BlueprintPure, Category = "Save")
	TArray<FName> GetLoadedSublevels() const { return LoadedSublevels; }

	void BindPlayerSaveHooks(APawn* Player);
	
	// All 20 hat assets registered here by the designer.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hats")
	TArray<TObjectPtr<UCatHatModifier>> AllHats;

	void UnlockHat(UCatHatModifier* Hat);
	
	UCatHatModifier* GetNextUnlockedHat(UCatHatModifier* CurrentHat) const;

	UCatHatModifier* GetEquippedHat() const;
	void SetEquippedHat(UCatHatModifier* Hat);

	
	UPROPERTY(BlueprintReadOnly, Category = "Save")
	TArray<FName> LoadedSublevels;

	UPROPERTY(BlueprintAssignable, Category = "Save")
	FOnWorldRestoreComplete OnWorldRestoreComplete;
private:
	static const FString SaveSlotName;
	static const int32 SaveUserIndex;

	UPROPERTY()
	TObjectPtr<UCatInventoryComponent> Inventory;

	UPROPERTY()
	TObjectPtr<UCatSaveGame> ActiveSave;
	

	bool bPendingWorldRestore = false;

	void PopulateSave(UCatSaveGame* Save);
	void ApplyNonWorldState(UCatSaveGame* Save);

	UFUNCTION()
	void OnQuestAccepted(UQuestData* Quest);

	UFUNCTION()
	void OnQuestCompleted(UQuestData* Quest);

	bool bIsLoading = false;

	TWeakObjectPtr<APlayerController> CachedPC;
};