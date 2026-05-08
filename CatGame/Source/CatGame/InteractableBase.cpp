#include "InteractableBase.h"

#include "QuestEffectComponent.h"

AInteractableBase::AInteractableBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AInteractableBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (SaveID.IsNone())
	{
		FVector Loc = GetActorLocation();
		uint32 Hash = HashCombine(GetTypeHash(Loc.X), HashCombine(GetTypeHash(Loc.Y), GetTypeHash(Loc.Z)));
		SaveID = FName(*FString::Printf(TEXT("%s_%u"), *GetClass()->GetName(), Hash));
	}
}

void AInteractableBase::Interact_Implementation(AActor* InteractInstigator)
{
	if (!CanInteract(InteractInstigator)) return;
	
	bOnCooldown = true;
	GetWorldTimerManager().SetTimer(CooldownTimer, [this]()
	{
		bOnCooldown = false;
	}, CooldownDuration, false);

	OnInteract(InteractInstigator);
}

bool AInteractableBase::CanInteract(AActor* InteractInstigator) const
{
	if (bOnCooldown)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s: CanInteract false - on cooldown"), *GetName()));
		return false;
	}

	if (UQuestEffectComponent* QFX = FindComponentByClass<UQuestEffectComponent>())
	{
		if (!QFX->QuestEffectTag.IsNone() && !QFX->bHasBeenTriggered)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s: CanInteract false - quest effect not triggered, tag: %s"), *GetName(), *QFX->QuestEffectTag.ToString()));
			return false;
		}
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("%s: CanInteract true"), *GetName()));
	return true;
}

void AInteractableBase::OnInteract_Implementation(AActor* InteractInstigator)
{
	// implemented in children
}

void AInteractableBase::ResetCooldown()
{
	bOnCooldown = false;
	GetWorldTimerManager().ClearTimer(CooldownTimer);
}

bool AInteractableBase::TestLog() const
{
	UE_LOG(LogTemp, Warning, TEXT("TESTLOG HIT"));
	return true;
}