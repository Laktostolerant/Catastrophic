#include "CatAppearanceModifier.h"

#if WITH_EDITOR
void UCatAppearanceModifier::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	for (const FName& Name : OwnedMorphTargets)
	{
		if (!Weights.Contains(Name))
			Weights.Add(Name, 0.f);
	}
	
	TArray<FName> ToRemove;
	for (const auto& Pair : Weights)
	{
		if (!OwnedMorphTargets.Contains(Pair.Key))
			ToRemove.Add(Pair.Key);
	}
	for (const FName& Name : ToRemove)
		Weights.Remove(Name);
}
#endif