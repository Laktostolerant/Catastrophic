#pragma once

#include "CoreMinimal.h"
#include "InteractableBase.h"
#include "CrawlSpace.generated.h"

UCLASS()
class CATGAME_API ACrawlSpace : public AInteractableBase
{
	GENERATED_BODY()

public:
	ACrawlSpace();

	UFUNCTION(BlueprintCallable, Category = "Crawl")
	bool GetCrawlPoints(FVector QueryLocation, FVector& OutEntry, FVector& OutExit);

	UPROPERTY(EditAnywhere, Category = "Crawl")
	bool bIsOneWay = false;

	UPROPERTY(EditAnywhere, Category = "Crawl")
	float MaxEntryDistance = 150.f;

protected:
	virtual void OnInteract_Implementation(AActor* InteractInstigator) override;

	UPROPERTY(VisibleAnywhere, Category = "Crawl")
	TObjectPtr<USceneComponent> CrawlPoint1;

	UPROPERTY(VisibleAnywhere, Category = "Crawl")
	TObjectPtr<USceneComponent> CrawlPoint2;

	UPROPERTY(VisibleAnywhere, Category = "Crawl")
	TObjectPtr<UStaticMeshComponent> CrawlMesh;

private:
	float CrawlDelayElapsed = 0.f;
	bool bCrawlExitPending = false;
};