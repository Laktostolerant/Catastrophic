#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CatPlayerController.generated.h"

class UCatHUDWidget;

UCLASS()
class CATGAME_API ACatPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UCatHUDWidget> HUDWidgetClass;

	UPROPERTY()
	TObjectPtr<UCatHUDWidget> HUDWidget;

public:
	UCatHUDWidget* GetHUDWidget() const { return HUDWidget; }
};