#pragma once

#include "CoreMinimal.h"
#include "CatAppearanceModifier.h"
#include "CatPostureModifier.generated.h"

UCLASS()
class UCatPostureModifier : public UCatAppearanceModifier
{
	GENERATED_BODY()
public:
	UCatPostureModifier()
	{
		OwnedMorphTargets = { "Back_Up", "Back_Arch" };
	}
};