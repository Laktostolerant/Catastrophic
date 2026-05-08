#pragma once

#include "CoreMinimal.h"
#include "CatAppearanceModifier.h"
#include "CatBodyMassModifier.generated.h"

UCLASS()
class UCatBodyMassModifier : public UCatAppearanceModifier
{
	GENERATED_BODY()
public:
	UCatBodyMassModifier()
	{
		OwnedMorphTargets = { "Legs_Thick", "Legs_Thin", "Belly_Fat", "Belly_Thin", "Neck_Thick", "Neck_Thin" };
	}
};