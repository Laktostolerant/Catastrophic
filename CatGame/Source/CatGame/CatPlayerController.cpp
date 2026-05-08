#include "CatPlayerController.h"
#include "CatHUDWidget.h"

void ACatPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UCatHUDWidget>(this, HUDWidgetClass);
		if (HUDWidget)
			HUDWidget->AddToViewport();
	}
}