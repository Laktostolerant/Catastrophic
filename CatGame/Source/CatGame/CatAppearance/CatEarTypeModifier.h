#pragma once

#include "CoreMinimal.h"
#include "CatAppearanceModifier.h"
#include "CatEarTypeModifier.generated.h"

/**
 * 
 */
UCLASS()
class UCatEarTypeModifier : public UCatAppearanceModifier
{
	GENERATED_BODY()
public:
	UCatEarTypeModifier()
	{
		OwnedMorphTargets = { "Ears_Round", "Ears_Pointy" };
	}
};