#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CatAppearanceModifier.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class UCatAppearanceModifier : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	TArray<FName> OwnedMorphTargets;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = -1.f, ClampMax = 1.f))
	TMap<FName, float> Weights;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
