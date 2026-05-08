#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CatCharacterMovementComponent.generated.h"

UCLASS()
class CATGAME_API UCatCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UCatCharacterMovementComponent();

	void NotifyJumpStart();

	UPROPERTY(EditAnywhere, Category = "EdgeDrop")
	float MinEdgeDropHeight = 80.f;

	UPROPERTY(EditAnywhere, Category = "EdgeDrop")
	float FrontPawCheckDistance = 15.f;

	UPROPERTY(EditAnywhere, Category = "EdgeDrop")
	float JumpEdgeIgnoreDuration = 0.35f;

	UPROPERTY(EditAnywhere, Category = "EdgeDrop")
	bool bDebugEdgeDetection = false;

	UPROPERTY(BlueprintReadWrite, Category = "Sprint")
	bool bIsSprinting = false;

	UPROPERTY(EditAnywhere, Category = "Sprint")
	float SprintSpeed = 350.f;

	UPROPERTY(EditAnywhere, Category = "Sprint")
	float DefaultWalkSpeed = 150.f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float LateralDampenStrengthWalk = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float LateralDampenStrengthSprint = 0.4f;
	
	UPROPERTY()
	float JumpEdgeIgnoreTimer = 0.f;

	virtual bool CanWalkOffLedges() const override;
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	bool IsLedgeAtDistance(float CheckDistance) const;
};