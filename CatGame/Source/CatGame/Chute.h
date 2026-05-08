#pragma once

#include "CoreMinimal.h"
#include "InteractableBase.h"
#include "Chute.generated.h"

UCLASS()
class CATGAME_API AChute : public AInteractableBase
{
	GENERATED_BODY()

public:
	AChute();

	// Returns false if blocked by one-way, too far, or missing points
	UFUNCTION(BlueprintCallable, Category = "Chute")
	bool GetChutePoints(FVector QueryLocation, FVector& OutEntry, FVector& OutExit, FVector& OutEntryForward);

	UPROPERTY(EditAnywhere, Category = "Chute")
	bool bIsOneWay = false;

	UPROPERTY(EditAnywhere, Category = "Chute")
	float MaxEntryDistance = 150.f;

	UPROPERTY(EditAnywhere, Category = "Chute")
	float EntryLerpDistance = 80.f;

	UPROPERTY(EditAnywhere, Category = "Chute")
	float EntryLerpDuration = 0.78f;

	UPROPERTY(EditAnywhere, Category = "Chute")
	float ExitLerpDistance = 80.f;

	UPROPERTY(EditAnywhere, Category = "Chute")
	float ExitLerpDuration = 1.0f;

protected:
	virtual void OnInteract_Implementation(AActor* InteractInstigator) override;

	UPROPERTY(VisibleAnywhere, Category = "Chute")
	TObjectPtr<USceneComponent> ChutePoint1;

	UPROPERTY(VisibleAnywhere, Category = "Chute")
	TObjectPtr<USceneComponent> ChutePoint2;

	UPROPERTY(VisibleAnywhere, Category = "Chute")
	TObjectPtr<UStaticMeshComponent> ChuteMesh;
};