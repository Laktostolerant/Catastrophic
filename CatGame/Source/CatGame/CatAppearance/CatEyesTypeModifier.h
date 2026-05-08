#pragma once

#include "CoreMinimal.h"
#include "CatAppearanceModifier.h"
#include "CatEyesTypeModifier.generated.h"

UCLASS()
class UCatEyesTypeModifier : public UCatAppearanceModifier
{
	GENERATED_BODY()
public:
	UCatEyesTypeModifier()
	{
		OwnedMorphTargets = { "Pupil Big", "Eyes_Tired" };
	}
};
