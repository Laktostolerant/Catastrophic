#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CatRespawnComponent.generated.h"

UCLASS(ClassGroup=Cat, meta=(BlueprintSpawnableComponent))
class CATGAME_API UCatRespawnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCatRespawnComponent();

	void NotifyLanded(const FHitResult& Hit, float ImpactVelocityZ);

	UFUNCTION(BlueprintCallable, Category="Respawn")
	void NotifyEnteredWater();
	void Respawn();

	UPROPERTY(EditAnywhere, Category="Respawn")
	float FallDeathVelocityThreshold = -2000.f;

	UPROPERTY(EditAnywhere, Category="Respawn")
	float SafeLocationUpdateInterval = 0.5f;

	// How far back from the edge the safe position is nudged
	UPROPERTY(EditAnywhere, Category="Respawn")
	float EdgeSafetyOffset = 80.f;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void TryUpdateSafeLocation();
	bool IsSafeLocation(const FVector& Location) const;
	bool IsStandingOnBlocker() const;

	FVector SafeLocation;
	FRotator SafeRotation;
	float SafeLocationTimer = 0.f;
};