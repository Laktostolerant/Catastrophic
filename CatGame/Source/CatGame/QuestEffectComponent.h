#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestEffectSubsystem.h"
#include "SaveableActor.h"
#include "QuestEffectComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuestEffectTriggered);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CATGAME_API UQuestEffectComponent : public UActorComponent, public ISaveableActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Quest")
	FName QuestEffectTag;

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestEffectTriggered OnQuestEffectTriggered;

	bool bHasBeenTriggered = false;


	UFUNCTION(BlueprintCallable, Category = "Quest")
	void TriggerEffect();
	
	
	virtual void SaveState(UCatSaveGame* SaveGame) override;
	virtual void RestoreState(const UCatSaveGame* SaveGame) override;
	virtual FName GetSaveID() const override { return QuestEffectTag; }
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};