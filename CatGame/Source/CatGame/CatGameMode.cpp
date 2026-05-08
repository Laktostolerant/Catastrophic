#include "CatGameMode.h"
#include "CatGameInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"

void ACatGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if (!NewPlayer) return;

	if (UCatGameInstance* GI = Cast<UCatGameInstance>(GetGameInstance()))
		GI->BindPlayerSaveHooks(NewPlayer->GetPawn());

	if (APawn* Pawn = NewPlayer->GetPawn())
	{
		Pawn->GetMovementComponent()->StopMovementImmediately();
		Cast<ACharacter>(Pawn)->GetCharacterMovement()->DisableMovement();
		NewPlayer->DisableInput(NewPlayer);
	}
}
