#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CatGameMode.generated.h"

UCLASS()
class CATGAME_API ACatGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
};