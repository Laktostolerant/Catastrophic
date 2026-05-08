#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"
#include "CatFurModifier.generated.h"

UCLASS()
class CATGAME_API UCatFurModifier : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fur")
	TObjectPtr<UMaterialInterface> FurMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fur")
	TObjectPtr<UMaterialInterface> BodyMaterial;
};