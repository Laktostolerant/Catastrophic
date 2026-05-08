#include "PushableComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"

void UPushableComponent::BeginPlay()
{
    Super::BeginPlay();
    SetPositionLocked(true);
}

UPrimitiveComponent* UPushableComponent::GetMeshComponent() const
{
    return Cast<UPrimitiveComponent>(GetOwner()->GetComponentByClass(UStaticMeshComponent::StaticClass()));
}

bool UPushableComponent::StartPush(FVector CatForward, FVector InCatLocationAtGrab)
{
    if (bIsPushed) return false;

    FVector BoxForward = GetOwner()->GetActorForwardVector();
    FVector BoxRight = GetOwner()->GetActorRightVector();

    float DotForward = FMath::Abs(FVector::DotProduct(CatForward, BoxForward));
    float DotRight = FMath::Abs(FVector::DotProduct(CatForward, BoxRight));

    if (FMath::Max(DotForward, DotRight) < PushAlignmentThreshold)
        return false;

    bIsPushed = true;

    if (DotForward >= DotRight)
    {
        float Sign = FMath::Sign(FVector::DotProduct(CatForward, BoxForward));
        LockedPushAxis = BoxForward * Sign;
    }
    else
    {
        float Sign = FMath::Sign(FVector::DotProduct(CatForward, BoxRight));
        LockedPushAxis = BoxRight * Sign;
    }

    BoxLocationAtGrab = GetOwner()->GetActorLocation();
    CatLocationAtGrab = InCatLocationAtGrab;
    EngageProgress = 0.f;
    bIsEngaging = true;

    UPrimitiveComponent* Mesh = GetMeshComponent();
    if (Mesh)
        Mesh->SetSimulatePhysics(false);

    SetPositionLocked(false);
    return true;
}

void UPushableComponent::StopPush()
{
    if (!bIsPushed) return;
    bIsPushed = false;

    UPrimitiveComponent* Mesh = GetMeshComponent();
    if (Mesh)
    {
        Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
        Mesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        Mesh->SetSimulatePhysics(true);
    }

    bIsEngaging = false;
    EngageProgress = 0.f;
    
    SetPositionLocked(true);
}

void UPushableComponent::TickPush(FVector CatLocation, float DeltaTime, bool bDebug)
{
    if (!bIsPushed) return;

    UPrimitiveComponent* Mesh = GetMeshComponent();
    if (!Mesh) return;

    if (bIsEngaging)
    {
        EngageProgress = FMath::Min(EngageProgress + DeltaTime, MinPushEngageDuration);
        if (EngageProgress >= MinPushEngageDuration)
            bIsEngaging = false;
    }

    float EngageAlpha = MinPushEngageDuration > 0.f
        ? FMath::Clamp(EngageProgress / MinPushEngageDuration, 0.f, 1.f)
        : 1.f;
    float EngageOffset = FMath::Lerp(0.f, MinPushEngageDistance, EngageAlpha);

    float CatDelta = FVector::DotProduct(CatLocation - CatLocationAtGrab, LockedPushAxis);
    CatDelta = FMath::Max(CatDelta, 0.f);

    FVector TargetLocation = BoxLocationAtGrab + LockedPushAxis * (CatDelta + EngageOffset);
    TargetLocation.Z = GetOwner()->GetActorLocation().Z;

    FVector Current = GetOwner()->GetActorLocation();
    FVector Smoothed = FMath::VInterpTo(Current, TargetLocation, DeltaTime, 25.f);
    Smoothed.Z = Current.Z;

    FVector MoveDir = Smoothed - Current;
    float MoveDist = MoveDir.Size();

    if (MoveDist > 0.1f)
    {
        FHitResult BlockHit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(GetOwner());

        bool bBlocked = GetOwner()->GetWorld()->SweepSingleByChannel(
            BlockHit,
            Current,
            Smoothed,
            GetOwner()->GetActorQuat(),
            ECC_WorldStatic,
            FCollisionShape::MakeBox(Mesh->Bounds.BoxExtent * 0.975f),
            Params
        );

        if (!bBlocked)
            GetOwner()->SetActorLocation(Smoothed, false);
    }

    if (bDebug)
    {
        DrawDebugSphere(GetOwner()->GetWorld(), TargetLocation, 12.f, 8, FColor::Yellow, false, -1.f);
        DrawDebugSphere(GetOwner()->GetWorld(), Current, 12.f, 8, FColor::Cyan, false, -1.f);
        DrawDebugLine(GetOwner()->GetWorld(), Current, Current + LockedPushAxis * 50.f, FColor::Red, false, -1.f, 0, 2.f);
    }
}

void UPushableComponent::SetPositionLocked(bool bLocked)
{
    UPrimitiveComponent* Mesh = GetMeshComponent();
    if (!Mesh) return;

    FBodyInstance* Body = Mesh->GetBodyInstance();
    if (!Body) return;

    Body->SetDOFLock(EDOFMode::Default);
    Body->bLockXTranslation = bLocked;
    Body->bLockYTranslation = bLocked;
    Body->CreateDOFLock();
}