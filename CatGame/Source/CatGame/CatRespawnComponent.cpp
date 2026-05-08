#include "CatRespawnComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RespawnBlockerComponent.h"
#include "Components/CapsuleComponent.h"

UCatRespawnComponent::UCatRespawnComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UCatRespawnComponent::BeginPlay()
{
    Super::BeginPlay();
    SafeLocation = GetOwner()->GetActorLocation();
    SafeRotation = GetOwner()->GetActorRotation();
}

void UCatRespawnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    SafeLocationTimer += DeltaTime;
    if (SafeLocationTimer >= SafeLocationUpdateInterval)
    {
        SafeLocationTimer = 0.f;
        TryUpdateSafeLocation();
    }
}

void UCatRespawnComponent::TryUpdateSafeLocation()
{
    ACharacter* Owner = Cast<ACharacter>(GetOwner());
    if (!Owner) return;

    if (IsStandingOnBlocker())
    {
      return;
    }

    if (!Owner->GetCharacterMovement()->IsMovingOnGround())
    {
       return;
    }

    FVector Loc = Owner->GetActorLocation();
    if (!IsSafeLocation(Loc))
    {
       return;
    }
    
    SafeLocation = Loc;
    SafeRotation = Owner->GetActorRotation();
}

bool UCatRespawnComponent::IsSafeLocation(const FVector& Location) const
{
    ACharacter* Owner = Cast<ACharacter>(GetOwner());
    if (!Owner) return false;

    static const FVector Dirs[] = {
        FVector(1,0,0), FVector(-1,0,0), FVector(0,1,0), FVector(0,-1,0)
    };

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    for (const FVector& Dir : Dirs)
    {
        FVector CheckStart = Location + Dir * EdgeSafetyOffset;
        FVector CheckEnd = CheckStart - FVector(0, 0, 150.f);
        FHitResult Hit;
        if (!GetWorld()->LineTraceSingleByChannel(Hit, CheckStart, CheckEnd, ECC_WorldStatic, Params))
            return false;
    }

    return true;
}

bool UCatRespawnComponent::IsStandingOnBlocker() const
{
    ACharacter* Owner = Cast<ACharacter>(GetOwner());
    if (!Owner) return false;

    FVector Start = Owner->GetActorLocation();
    FVector End = Start - FVector(0, 0, Owner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 10.f);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
    {
        return Hit.GetActor() && Hit.GetActor()->FindComponentByClass<URespawnBlockerComponent>();
    }

    return false;
}

void UCatRespawnComponent::NotifyLanded(const FHitResult& Hit, float ImpactVelocityZ)
{
    if (ImpactVelocityZ < FallDeathVelocityThreshold)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("FALL DEATH TRIGGERED"));
        Respawn();
    }
}

void UCatRespawnComponent::NotifyEnteredWater()
{
    Respawn();
}

void UCatRespawnComponent::Respawn()
{
    ACharacter* Owner = Cast<ACharacter>(GetOwner());
    if (!Owner) return;

    Owner->GetCharacterMovement()->StopMovementImmediately();
    Owner->SetActorLocation(SafeLocation, false, nullptr, ETeleportType::TeleportPhysics);
    Owner->SetActorRotation(SafeRotation);
}