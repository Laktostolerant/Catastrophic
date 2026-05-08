#pragma once

#include "CoreMinimal.h"
#include "CatAppearanceModifier.h"
#include "CatHeadTypeModifier.generated.h"

UCLASS()
class UCatHeadTypeModifier : public UCatAppearanceModifier
{
	GENERATED_BODY()
public:
	UCatHeadTypeModifier()
	{
		OwnedMorphTargets = { "Jaw_Big", "Head_Square", "Head_Round", "Cheeks_Pointy", "Cheeks_Off" };
	}
};
