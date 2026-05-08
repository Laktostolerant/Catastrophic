#include "CrawlSpace.h"
#include "Components/SceneComponent.h"

ACrawlSpace::ACrawlSpace()
{
	PrimaryActorTick.bCanEverTick = false;

	CrawlMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrawlMesh"));
	SetRootComponent(CrawlMesh);

	CrawlPoint1 = CreateDefaultSubobject<USceneComponent>(TEXT("CrawlPoint1"));
	CrawlPoint1->SetupAttachment(CrawlMesh);

	CrawlPoint2 = CreateDefaultSubobject<USceneComponent>(TEXT("CrawlPoint2"));
	CrawlPoint2->SetupAttachment(CrawlMesh);
}

bool ACrawlSpace::GetCrawlPoints(FVector QueryLocation, FVector& OutEntry, FVector& OutExit)
{
	if (!CrawlPoint1 || !CrawlPoint2)
		return false;

	FVector P1 = CrawlPoint1->GetComponentLocation();
	FVector P2 = CrawlPoint2->GetComponentLocation();

	float DistSqToP1 = FVector::DistSquared(QueryLocation, P1);
	float DistSqToP2 = FVector::DistSquared(QueryLocation, P2);
	bool bP1IsNearer = DistSqToP1 <= DistSqToP2;

	if (bIsOneWay && !bP1IsNearer)
		return false;

	if (MaxEntryDistance > 0.f)
	{
		float NearestDistSq = bP1IsNearer ? DistSqToP1 : DistSqToP2;
		if (NearestDistSq > FMath::Square(MaxEntryDistance))
			return false;
	}

	OutEntry = bP1IsNearer ? P1 : P2;
	OutExit = bP1IsNearer ? P2 : P1;

	return true;
}

void ACrawlSpace::OnInteract_Implementation(AActor* InteractInstigator)
{

}