#include "CatCharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "CatCharacter.h"

UCatCharacterMovementComponent::UCatCharacterMovementComponent()
{
    // MaxWalkSpeed = 150.f;
}

bool UCatCharacterMovementComponent::IsLedgeAtDistance(float CheckDistance) const
{
    if (!CharacterOwner || !UpdatedComponent)
        return false;

    const FVector ActorLoc = UpdatedComponent->GetComponentLocation();
    const FVector Forward = UpdatedComponent->GetForwardVector();
    const float CapsuleHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

    const FVector CheckStart = ActorLoc + Forward * CheckDistance;
    const FVector CheckEnd = CheckStart - FVector(0.f, 0.f, CapsuleHalfHeight + MinEdgeDropHeight);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    FHitResult Hit;
    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, CheckStart, CheckEnd, ECC_Visibility, Params);

    bool bIsLedge = true;
    if (bHit)
    {
        float Drop = CheckStart.Z - Hit.ImpactPoint.Z;
        bIsLedge = Drop >= MinEdgeDropHeight;
    }

    if (bDebugEdgeDetection)
    {
        DrawDebugLine(GetWorld(), CheckStart, CheckEnd,
            bIsLedge ? FColor::Red : FColor::Green, false, 0.f, 0, 2.f);
        DrawDebugSphere(GetWorld(), CheckStart, 6.f, 8,
            bIsLedge ? FColor::Red : FColor::Green, false, 0.f);
    }

    return bIsLedge;
}

void UCatCharacterMovementComponent::NotifyJumpStart()
{
    JumpEdgeIgnoreTimer = JumpEdgeIgnoreDuration;
}

bool UCatCharacterMovementComponent::CanWalkOffLedges() const
{
    if (!Super::CanWalkOffLedges())
        return false;

    if (JumpEdgeIgnoreTimer > 0.f)
        return true;

    if (bIsSprinting)
        return true;

    return !IsLedgeAtDistance(FrontPawCheckDistance);
}

void UCatCharacterMovementComponent::CalcVelocity(
    float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration)
{
    Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);

    if (JumpEdgeIgnoreTimer > 0.f)
        return;

    if (!IsMovingOnGround())
        return;

    // Ledge blocking for walking
    if (!bIsSprinting && IsLedgeAtDistance(FrontPawCheckDistance))
    {
        const FVector Forward = UpdatedComponent->GetForwardVector();
        const float ForwardSpeed = FVector::DotProduct(Velocity, Forward);
        if (ForwardSpeed > 0.f)
            Velocity -= Forward * ForwardSpeed;
    }

    // Lateral dampening based on actual speed
    const float CurrentSpeed = Velocity.Size2D();
    if (CurrentSpeed < KINDA_SMALL_NUMBER)
        return;

    const FVector Forward = UpdatedComponent->GetForwardVector();
    const FVector VelDir = Velocity.GetSafeNormal2D();
    const float ForwardDot = FMath::Clamp(FVector::DotProduct(Forward, VelDir), 0.f, 1.f);

    // t=0 at walk speed, t=1 at sprint speed, blend dampen strength by actual speed
    const float SpeedT = FMath::Clamp((CurrentSpeed - DefaultWalkSpeed) / FMath::Max(SprintSpeed - DefaultWalkSpeed, 1.f), 0.f, 1.f);
    const float DampenStrength = FMath::Lerp(LateralDampenStrengthWalk, LateralDampenStrengthSprint, SpeedT);

    const float LateralScale = FMath::Lerp(DampenStrength, 1.f, ForwardDot);

    const FVector ForwardVel = Forward * FVector::DotProduct(Velocity, Forward);
    const FVector LateralVel = Velocity - ForwardVel;
    Velocity = ForwardVel + LateralVel * LateralScale;
}

void UCatCharacterMovementComponent::TickComponent(
    float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (JumpEdgeIgnoreTimer > 0.f)
        JumpEdgeIgnoreTimer -= DeltaTime;

    if (bDebugEdgeDetection)
    {
        GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::White,
            FString::Printf(TEXT("Sprinting: %d | JumpTimer: %.2f | FrontLedge: %d"),
            bIsSprinting ? 1 : 0,
            JumpEdgeIgnoreTimer,
            IsLedgeAtDistance(FrontPawCheckDistance) ? 1 : 0));
    }
}