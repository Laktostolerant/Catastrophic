#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CatInventoryComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CATGAME_API UCatInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddItem(FName ItemID, int32 Qty = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(FName ItemID, int32 Qty = 1);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(FName ItemID, int32 Qty = 1) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetItemCount(FName ItemID) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void Clear();
	
	const TMap<FName, int32>& GetAllItems() const { return Items; }


	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;
	
	
private:
	UPROPERTY()
	TMap<FName, int32> Items;
};