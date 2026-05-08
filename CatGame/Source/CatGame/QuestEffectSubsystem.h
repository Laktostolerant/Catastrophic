// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "QuestEffectSubsystem.generated.h"

/**
 * 
 */

DECLARE_DELEGATE(FQuestEffectDelegate);

UCLASS()
class CATGAME_API UQuestEffectSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void RegisterListener(FName EffectTag, FQuestEffectDelegate Callback);
	
	UFUNCTION(BlueprintCallable, Category = "Quest Effects")
	void BroadcastEffect(FName EffectTag);

private:
	TMap<FName, TArray<FQuestEffectDelegate>> Listeners;
};
