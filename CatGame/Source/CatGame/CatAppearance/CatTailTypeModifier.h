#pragma once

#include "CoreMinimal.h"
#include "CatAppearanceModifier.h"
#include "CatTailTypeModifier.generated.h"

UCLASS()
class UCatTailTypeModifier : public UCatAppearanceModifier
{
	GENERATED_BODY()
public:
	UCatTailTypeModifier()
	{
		OwnedMorphTargets = { "Tail_Thick" };
	}
};
