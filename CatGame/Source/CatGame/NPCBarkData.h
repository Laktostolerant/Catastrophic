#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NPCBarkData.generated.h"

UCLASS(BlueprintType)
class CATGAME_API UNPCBarkData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TArray<FText> BarkLines;
};