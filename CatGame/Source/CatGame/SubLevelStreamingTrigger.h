#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SubLevelStreamingTrigger.generated.h"

UCLASS()
class CATGAME_API ASubLevelStreamingTrigger : public AActor
{
	GENERATED_BODY()

public:
	ASubLevelStreamingTrigger();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnLevelLoaded();

	UFUNCTION()
	void OnLevelUnloaded();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level Streaming", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UBoxComponent> TriggerVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Streaming", meta = (AllowPrivateAccess = "true"))
	FName LevelToLoad;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Streaming", meta = (AllowPrivateAccess = "true"))
	FName LevelToUnload;

	UPROPERTY(EditAnywhere, Category = "Level Streaming", meta = (AllowPrivateAccess = "true"))
	bool bMakeVisibleAfterLoad = true;

	bool bTriggered = false;

	void DoLoad();
	void DoUnload();
};