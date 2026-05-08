#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "InteractableBase.generated.h"

UCLASS(Abstract)
class CATGAME_API AInteractableBase : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    AInteractableBase();
    void OnConstruction(const FTransform& Transform);

    virtual void Interact_Implementation(AActor* InteractInstigator) override final;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    virtual bool CanInteract(AActor* InteractInstigator) const;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool TestLog() const;

    UPROPERTY(VisibleAnywhere, Category = "Save")
    FName SaveID;
    
protected:
    UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
    void OnInteract(AActor* InteractInstigator);

    UPROPERTY(EditAnywhere, Category = "Interaction")
    float CooldownDuration = 1.0f;

    void ResetCooldown();

private:
    bool bOnCooldown = false;
    FTimerHandle CooldownTimer;
};