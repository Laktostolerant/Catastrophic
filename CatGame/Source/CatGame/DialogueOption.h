#pragma once

#include "CoreMinimal.h"
#include "DialogueOption.generated.h"

USTRUCT(BlueprintType)
struct FDialogueOption
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FText Label;

	UPROPERTY(BlueprintReadOnly)
	FName OptionID;
};