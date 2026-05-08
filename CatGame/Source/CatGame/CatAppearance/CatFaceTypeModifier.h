#pragma once

#include "CoreMinimal.h"
#include "CatAppearanceModifier.h"
#include "CatFaceTypeModifier.generated.h"

UCLASS()
class UCatFaceTypeModifier : public UCatAppearanceModifier
{
	GENERATED_BODY()
public:
	UCatFaceTypeModifier()
	{
		OwnedMorphTargets = { "Nose_Wide", "Nose_Narrow", "Snout_Up", "Snout_Big" };
	}
};
