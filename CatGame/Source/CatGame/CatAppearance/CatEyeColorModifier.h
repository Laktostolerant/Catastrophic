#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"
#include "CatEyeColorModifier.generated.h"

UCLASS()
class CATGAME_API UCatEyeColorModifier : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Eyes")
	TObjectPtr<UMaterialInterface> EyesMaterial;
};