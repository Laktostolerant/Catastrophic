#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PushableComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CATGAME_API UPushableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pushing")
	float PushSpeedMultiplier = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pushing")
	float PushAlignmentThreshold = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pushing")
	float MinPushEngageDistance = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pushing")
	float MinPushEngageDuration = 0.15f;
	
	bool StartPush(FVector CatForward, FVector CatLocationAtGrab);
	void StopPush();
	void TickPush(FVector CatLocation, float DeltaTime, bool bDebug = false);

	bool IsBeingPushed() const { return bIsPushed; }

	FVector LockedPushAxis = FVector::ZeroVector;

protected:
	virtual void BeginPlay() override;
	
private:
	bool bIsPushed = false;
	FVector BoxLocationAtGrab = FVector::ZeroVector;
	FVector CatLocationAtGrab = FVector::ZeroVector;
	UPrimitiveComponent* GetMeshComponent() const;

	void SetPositionLocked(bool bLocked);

	float EngageProgress = 0.f;
	bool bIsEngaging = false;
};