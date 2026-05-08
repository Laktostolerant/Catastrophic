#include "Chute.h"
#include "Components/SceneComponent.h"

AChute::AChute()
{
	PrimaryActorTick.bCanEverTick = false;

	ChuteMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChuteMesh"));
	SetRootComponent(ChuteMesh);

	ChutePoint1 = CreateDefaultSubobject<USceneComponent>(TEXT("ChutePoint1"));
	ChutePoint1->SetupAttachment(ChuteMesh);

	ChutePoint2 = CreateDefaultSubobject<USceneComponent>(TEXT("ChutePoint2"));
	ChutePoint2->SetupAttachment(ChuteMesh);
}

bool AChute::GetChutePoints(FVector QueryLocation, FVector& OutEntry, FVector& OutExit, FVector& OutEntryForward)
{
	if (!ChutePoint1 || !ChutePoint2)
		return false;

	FVector P1 = ChutePoint1->GetComponentLocation();
	FVector P2 = ChutePoint2->GetComponentLocation();

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
	
	FVector Horizontal = OutExit - OutEntry;
	Horizontal.Z = 0.f;
	OutEntryForward = Horizontal.GetSafeNormal();

	return true;
}

void AChute::OnInteract_Implementation(AActor* InteractInstigator)
{

}