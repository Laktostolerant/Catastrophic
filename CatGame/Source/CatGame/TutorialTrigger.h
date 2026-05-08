#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TutorialTrigger.generated.h"

class UBoxComponent;

UCLASS()
class CATGAME_API ATutorialTrigger : public AActor
{
	GENERATED_BODY()

public:
	ATutorialTrigger();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial")
	FText PromptText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial")
	FVector2D WidgetScreenPosition = FVector2D(0.5f, 0.8f);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* TriggerBox;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial")
	void OnPlayerEnter();

	UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial")
	void OnPlayerExit();
};