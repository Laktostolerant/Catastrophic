#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/StaticMesh.h"
#include "CatHatModifier.generated.h"

UCLASS(BlueprintType)
class CATGAME_API UCatHatModifier : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hat")
	FName HatID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hat")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hat")
	TObjectPtr<UStaticMesh> HatMesh;

	// automatically sockets the hats to this, its a custom socket i added to the mesh of the cat
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hat")
	FName AttachSocket = FName("hat_socket");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hat")
	FVector RelativeOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hat")
	FRotator RelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hat")
	FVector RelativeScale = FVector(1.f, 1.f, 1.f);
};