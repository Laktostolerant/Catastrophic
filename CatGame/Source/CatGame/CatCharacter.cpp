#include "CatCharacter.h"
#include "CatCharacterMovementComponent.h"
#include "CatGameInstance.h"
#include "CatAppearance/CatHatModifier.h"
#include "InteractableBase.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"

ACatCharacter::ACatCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UCatCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
    PrimaryActorTick.bCanEverTick = true;

    GetCharacterMovement()->bCanWalkOffLedges = false;
    GetCharacterMovement()->LedgeCheckThreshold = 0.f;

    RespawnComponent = CreateDefaultSubobject<UCatRespawnComponent>(TEXT("RespawnComponent"));
    AppearanceComponent = CreateDefaultSubobject<UCatAppearanceComponent>(TEXT("AppearanceComponent"));
}

void ACatCharacter::BeginPlay()
{
    Super::BeginPlay();

    GetWorld()->GetWorldSettings()->TimeDilation = 1.f;

    AppearanceComponent->ApplyToMesh(GetMesh());
}

void ACatCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsOnCooldown)
    {
        CooldownTimer -= DeltaTime;
        if (CooldownTimer <= 0.f)
        {
            bIsOnCooldown = false;
            CooldownTimer = 0.f;
        }
    }

    float CurrentYaw = GetActorRotation().Yaw;
    float RawDelta = FMath::FindDeltaAngleDegrees(LastYaw, CurrentYaw);
    float RawTurnRate = (DeltaTime > KINDA_SMALL_NUMBER) ? -(RawDelta / DeltaTime) : 0.f;
    SmoothedTurnRate = FMath::FInterpTo(SmoothedTurnRate, RawTurnRate, DeltaTime, TurnRateInterpSpeed);
    LastYaw = CurrentYaw;

    TickVault(DeltaTime);
    TickCrawl(DeltaTime);
    TickChuteCrawl(DeltaTime);
    TickJump(DeltaTime);
    TickEdgeDrop(DeltaTime);
    TickSlope(DeltaTime);
    TickPush(DeltaTime);
    UpdateVaultDetection();
}

void ACatCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

    if (PrevMovementMode == MOVE_Walking && GetCharacterMovement()->IsFalling())
    {
        if (bIsJumping || bIsEdgeDropping || bIsFreeFalling)
            return;

        bIsFreeFalling = true;
        if (FallingMontage)
            PlayAnimMontage(FallingMontage, 1.0f);
    }
}

bool ACatCharacter::CanVaultWithMinHeight() const
{
    if (!bCanVault)
        return false;

    float ClimbHeight = VaultTargetLocation.Z - GetActorLocation().Z;
    return ClimbHeight >= MinVaultHeight;
}

void ACatCharacter::ResetCatJumpState()
{
    bIsJumping = false;
    AirborneTicks = 0;
    bPassedLandGate = false;
    ActiveLandGatePos = 0.f;
    GetWorldTimerManager().ClearTimer(JumpTakeoffTimerHandle);

    if (ActiveJumpMontage)
        StopAnimMontage(ActiveJumpMontage);
}

void ACatCharacter::ResetEdgeDropState()
{
    bIsEdgeDropping = false;
    bIsFreeFalling = false;
    bPassedFallLandGate = false;

    if (FallLandMontage)
        StopAnimMontage(FallLandMontage);
    if (FallingMontage)
        StopAnimMontage(FallingMontage);
    if (EdgeExitMontage)
        StopAnimMontage(EdgeExitMontage);
}

void ACatCharacter::TickVault(float DeltaTime)
{
    if (!bIsVaulting)
        return;

    VaultAccumulator += DeltaTime;

    if (VaultAccumulator < VaultPhase1Duration)
    {
        float Alpha = FMath::Clamp(VaultAccumulator / VaultPhase1Duration, 0.f, 1.f);
        SetActorLocation(FMath::Lerp(VaultOriginLocation, VaultIntermediateLocation, Alpha));
    }
    else
    {
        float Phase2Elapsed = VaultAccumulator - VaultPhase1Duration;
        float Alpha = FMath::Clamp(Phase2Elapsed / VaultPhase2Duration, 0.f, 1.f);
        SetActorLocation(FMath::Lerp(VaultIntermediateLocation, VaultFinalLocation, Alpha));

        if (Alpha >= 1.f)
        {
            bIsVaulting = false;
            VaultAccumulator = 0.f;

            if (bDebugVaultSlowMotion)
                GetWorld()->GetWorldSettings()->TimeDilation = 1.f;

            OnVaultMovementComplete();
        }
    }
}

void ACatCharacter::TickCrawl(float DeltaTime)
{
    if (!bIsCrawling)
        return;

    // Entry delay, hold at entry point
    if (CrawlDelayElapsed < CrawlEntryDelay)
    {
        CrawlDelayElapsed += DeltaTime;
        return;
    }

    // Exit delay, hold at exit point
    if (bCrawlExitPending)
    {
        CrawlDelayElapsed += DeltaTime;
        if (CrawlDelayElapsed >= CrawlExitDelay)
        {
            bIsCrawling = false;
            GetCharacterMovement()->SetMovementMode(MOVE_Walking);
            ReattachCamera();
            OnCrawlComplete();
        }
        return;
    }

    CrawlElapsed += DeltaTime;
    float Alpha = FMath::Clamp(CrawlElapsed / CrawlDuration, 0.f, 1.f);
    SetActorLocation(FMath::Lerp(CrawlEntry, CrawlExit, Alpha));

    if (Alpha >= 1.f)
    {
        bCrawlExitPending = true;
        CrawlDelayElapsed = 0.f;
    }
}

void ACatCharacter::TickEdgeDrop(float DeltaTime)
{
    if (!bIsEdgeDropping && !bIsFreeFalling)
        return;

    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    if (!AnimInst)
        return;

    if (bIsEdgeDropping)
    {
        EdgeDropAccumulator += DeltaTime;
        float Alpha = FMath::Clamp(EdgeDropAccumulator / EdgeDropForwardDuration, 0.f, 1.f);
        FVector NewLoc = FMath::Lerp(EdgeDropOrigin, EdgeDropTarget, Alpha);
        NewLoc.Z = GetActorLocation().Z;
        SetActorLocation(NewLoc);

        if (EdgeExitMontage && AnimInst->Montage_IsPlaying(EdgeExitMontage))
            return;

        bIsEdgeDropping = false;
        bIsFreeFalling = true;
        GetCharacterMovement()->SetMovementMode(MOVE_Falling);

        FVector Forward = GetActorForwardVector();
        LaunchCharacter(
            FVector(Forward.X * EdgeDropExitForwardVelocity, Forward.Y * EdgeDropExitForwardVelocity, -EdgeDropExitDownVelocity),
            true, true);

        if (FallingMontage)
            PlayAnimMontage(FallingMontage, 1.0f);

        return;
    }

    if (bIsFreeFalling)
    {
        if (!bPassedFallLandGate)
        {
            FHitResult LandHit;
            FVector Start = GetActorLocation();
            FVector End = Start - FVector(0, 0, FallLandDetectionDistance);
            FCollisionQueryParams Params;
            Params.AddIgnoredActor(this);

            bool bNearGround = GetWorld()->LineTraceSingleByChannel(
                LandHit, Start, End, ECC_WorldStatic, Params);

            if (bNearGround && GetVelocity().Z < 0.f)
            {
                bPassedFallLandGate = true;

                if (FallingMontage)
                    StopAnimMontage(FallingMontage);
                if (FallLandMontage)
                    PlayAnimMontage(FallLandMontage, 1.0f);
            }
            return;
        }

        if (FallLandMontage && AnimInst->GetInstanceForMontage(FallLandMontage))
            return;

        ResetEdgeDropState();
    }
}

void ACatCharacter::TickJump(float DeltaTime)
{
    if (!bIsJumping || !ActiveJumpMontage)
        return;

    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    if (!AnimInst)
        return;

    FAnimMontageInstance* MontageInst = AnimInst->GetInstanceForMontage(ActiveJumpMontage);
    if (!MontageInst)
    {
        ResetCatJumpState();
        return;
    }

    float VelZ = GetVelocity().Z;
    float CurrentPos = MontageInst->GetPosition();

    if (AirborneTicks == 0 && VelZ > 10.f)
        AirborneTicks = 1;
    else if (AirborneTicks > 0)
        AirborneTicks++;

    if (AirborneTicks == 0)
        return;

    if (AirborneTicks >= MinAirborneTicks && bHasPotentialVault && !bIsOnCooldown
        && VelZ > MidAirVaultMinVelZ && GetCharacterMovement()->IsFalling())
    {
        float HeightDiff = PotentialVaultTarget.Z - GetActorLocation().Z;

        if (HeightDiff >= -LedgeGrabMaxHeight && HeightDiff <= MaxClimbHeight)
        {
            VaultTargetLocation = PotentialVaultTarget;
            WallHitLocation = PotentialWallHitLocation;
            WallHitNormal = PotentialWallHitNormal;
            bCanVault = true;

            if (FMath::Abs(HeightDiff) <= LedgeGrabMaxHeight)
                ExecuteLedgeGrab();
            else
                ExecuteVault();

            return;
        }
    }

    if (!bPassedLandGate)
    {
        if (CurrentPos >= ActiveLandGatePos - 0.01f)
        {
            FHitResult LandHit;
            FVector Start = GetActorLocation();
            FVector End = Start - FVector(0.f, 0.f, LandingDetectionDistance);
            FCollisionQueryParams Params;
            Params.AddIgnoredActor(this);

            bool bRaycastHit = GetWorld()->LineTraceSingleByChannel(LandHit, Start, End, ECC_WorldStatic, Params);

            if (bDebugJumpRaycast)
            {
                DrawDebugLine(GetWorld(), Start, End, bRaycastHit ? FColor::Green : FColor::Red, false, -1.f, 0, 2.f);
                if (bRaycastHit)
                    DrawDebugSphere(GetWorld(), LandHit.ImpactPoint, 8.f, 8, FColor::Green, false, -1.f);
            }

            if (bRaycastHit)
            {
                bPassedLandGate = true;
                MontageInst->SetPosition(ActiveLandGatePos);
                MontageInst->SetPlayRate(1.f);
            }
            else
            {
                MontageInst->SetPosition(ActiveLandGatePos);
                MontageInst->SetPlayRate(0.00001f);
            }
        }
        return;
    }

    if (!MontageInst || CurrentPos >= ActiveJumpMontage->GetPlayLength() - 0.05f)
        ResetCatJumpState();
}

void ACatCharacter::TickSlope(float DeltaTime)
{
    FVector ActorLoc = GetActorLocation();
    FVector CatForward = GetActorForwardVector();
    FCollisionQueryParams SlopeParams;
    SlopeParams.AddIgnoredActor(this);

    FHitResult FrontHit, RearHit;
    FVector FrontStart = ActorLoc + CatForward * BodyHalfLength;
    FVector RearStart = ActorLoc - CatForward * BodyHalfLength;
    FVector TraceDown = FVector(0, 0, -100.f);

    bool bFrontHit = GetWorld()->LineTraceSingleByChannel(FrontHit, FrontStart, FrontStart + TraceDown, ECC_WorldStatic, SlopeParams);
    bool bRearHit = GetWorld()->LineTraceSingleByChannel(RearHit, RearStart, RearStart + TraceDown, ECC_WorldStatic, SlopeParams);

    if (bDebugSlopeTraces)
    {
        DrawDebugLine(GetWorld(), FrontStart, FrontStart + TraceDown, FColor::Green, false, -1.f, 0, 1.f);
        DrawDebugLine(GetWorld(), RearStart, RearStart + TraceDown, FColor::Red, false, -1.f, 0, 1.f);
        if (bFrontHit) DrawDebugSphere(GetWorld(), FrontHit.ImpactPoint, 5.f, 8, FColor::Green, false, -1.f);
        if (bRearHit) DrawDebugSphere(GetWorld(), RearHit.ImpactPoint, 5.f, 8, FColor::Red, false, -1.f);
    }

    const float MinWalkableDot = FMath::Cos(FMath::DegreesToRadians(GetCharacterMovement()->GetWalkableFloorAngle()));

    const bool bFrontWalkable = bFrontHit && FVector::DotProduct(FrontHit.ImpactNormal, FVector::UpVector) >= MinWalkableDot;
    const bool bRearWalkable  = bRearHit  && FVector::DotProduct(RearHit.ImpactNormal,  FVector::UpVector) >= MinWalkableDot;
    
    if (GetCharacterMovement()->IsMovingOnGround() && bFrontWalkable && bRearWalkable)
    {
        SmoothedFrontNormal = FMath::VInterpTo(SmoothedFrontNormal, FrontHit.ImpactNormal, DeltaTime, SlopeNormalInterpSpeed);
        SmoothedRearNormal  = FMath::VInterpTo(SmoothedRearNormal,  RearHit.ImpactNormal,  DeltaTime, SlopeNormalInterpSpeed);

        FVector AveragedNormal = (SmoothedFrontNormal + SmoothedRearNormal).GetSafeNormal();
        float SlopePitch = FMath::RadiansToDegrees(FMath::Atan2(AveragedNormal.Z, FVector::DotProduct(AveragedNormal, CatForward))) - 90.f;

        FRotator TargetRotation = FRotator(0.f, 270.f, -SlopePitch);
        FRotator CurrentRotation = GetMesh()->GetRelativeRotation();
        FRotator SmoothedRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, SlopeRotationInterpSpeed);
        GetMesh()->SetRelativeRotation(SmoothedRotation);

        float SlopeCorrection = SlopePitch > 0.f ? MeshSlopeZCorrectionUp : MeshSlopeZCorrectionDown;
        float ZOffset = MeshOffset.Z + FMath::Sin(FMath::DegreesToRadians(SlopePitch)) * SlopeCorrection;
        GetMesh()->SetRelativeLocation(FVector(MeshOffset.X, MeshOffset.Y, ZOffset));
    }
    else
    {
        SmoothedFrontNormal = FMath::VInterpTo(SmoothedFrontNormal, FVector::UpVector, DeltaTime, SlopeNormalInterpSpeed);
        SmoothedRearNormal = FMath::VInterpTo(SmoothedRearNormal, FVector::UpVector, DeltaTime, SlopeNormalInterpSpeed);

        FRotator CurrentRotation = GetMesh()->GetRelativeRotation();
        FRotator SmoothedRotation = FMath::RInterpTo(CurrentRotation, FRotator(0.f, 270.f, 0.f), DeltaTime, SlopeRotationInterpSpeed);
        GetMesh()->SetRelativeRotation(SmoothedRotation);
        GetMesh()->SetRelativeLocation(MeshOffset);
    }
}

void ACatCharacter::TickPush(float DeltaTime)
{
    UCatCharacterMovementComponent* CatMove =
        Cast<UCatCharacterMovementComponent>(GetCharacterMovement());

    auto ReleasePushable = [&]()
    {
        PushReleaseFrames = 0;
        if (ActivePushable)
        {
            ActivePushable->StopPush();
            ActivePushable = nullptr;
            if (CatMove)
                CatMove->MaxWalkSpeed = CatMove->bIsSprinting
                    ? CatMove->SprintSpeed
                    : CatMove->DefaultWalkSpeed;
        }
    };

    if (bIsJumping || bIsVaulting || bIsEdgeDropping || bIsFreeFalling || bIsCrawling)
    {
        ReleasePushable();
        return;
    }

    if (!GetCharacterMovement()->IsMovingOnGround())
    {
        ReleasePushable();
        return;
    }

    FVector Start = GetActorLocation();
    FVector Forward = GetActorForwardVector();
    FVector End = Start + Forward * PushTraceDistance;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(PushTraceRadius);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    FHitResult Hit;
    bool bHit = GetWorld()->SweepSingleByChannel(
        Hit, Start, End, FQuat::Identity, ECC_WorldDynamic, Sphere, Params);

    if (!bHit)
    {
        ReleasePushable();
        if (bDebugPushTrace)
            DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, -1.f, 0, 1.f);
        return;
    }

    UPushableComponent* Pushable = Hit.GetActor()
        ? Hit.GetActor()->FindComponentByClass<UPushableComponent>()
        : nullptr;

    if (!Pushable)
    {
        ReleasePushable();
        if (bDebugPushTrace)
            DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, -1.f, 0, 1.f);
        return;
    }

    // compute corner check values
    FVector LocalHit = Hit.GetActor()->GetActorTransform().InverseTransformPosition(Hit.ImpactPoint);
    FVector ToBox = (Hit.GetActor()->GetActorLocation() - GetActorLocation());
    ToBox.Z = 0.f;
    FVector ToBoxDir = ToBox.GetSafeNormal();
    FVector BoxForward = Hit.GetActor()->GetActorForwardVector();
    FVector BoxRight = Hit.GetActor()->GetActorRightVector();
    float DotF = FMath::Abs(FVector::DotProduct(ToBoxDir, BoxForward));
    float DotR = FMath::Abs(FVector::DotProduct(ToBoxDir, BoxRight));
    float PerpLocalOffset = DotF >= DotR ? FMath::Abs(LocalHit.Y) : FMath::Abs(LocalHit.X);
    FVector BandAxis = DotF >= DotR ? BoxRight : BoxForward;

    if (bDebugPushTrace)
    {
        DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, -1.f, 0, 1.f);
        DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 6.f, 8,
            PerpLocalOffset <= PushCenterBandHalfWidth ? FColor::Green : FColor::Red, false, -1.f);

        FVector FaceCenter = Hit.ImpactPoint - BandAxis * FVector::DotProduct(
            Hit.ImpactPoint - Hit.GetActor()->GetActorLocation(), BandAxis);
        DrawDebugLine(GetWorld(),
            FaceCenter - BandAxis * PushCenterBandHalfWidth,
            FaceCenter + BandAxis * PushCenterBandHalfWidth,
            PerpLocalOffset <= PushCenterBandHalfWidth ? FColor::Green : FColor::Red, false, -1.f, 0, 3.f);

        if (ActivePushable)
            DrawDebugLine(GetWorld(), Hit.ImpactPoint,
                Hit.ImpactPoint + ActivePushable->LockedPushAxis * 60.f,
                FColor::Yellow, false, -1.f, 0, 2.f);

        const bool bInBand = PerpLocalOffset <= PushCenterBandHalfWidth;
        const float Speed2D = GetVelocity().Size2D();
        const FVector ToObject = (Hit.GetActor()->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
        const float Dot = FVector::DotProduct(GetVelocity().GetSafeNormal2D(), ToObject);

        GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::White,
            FString::Printf(TEXT("InBand:%d  Perp:%.1f / %.1f  Speed:%.1f  Dot:%.2f  Active:%d  ReleaseFrames:%d"),
            bInBand ? 1 : 0,
            PerpLocalOffset, PushCenterBandHalfWidth,
            Speed2D, Dot,
            ActivePushable ? 1 : 0,
            PushReleaseFrames));
    }

    // hard stop on corners
    if (PerpLocalOffset > PushCenterBandHalfWidth)
    {
        ReleasePushable();
        return;
    }

    PushReleaseFrames = 0;

    if (ActivePushable != Pushable || ActivePushable == nullptr)
    {
        if (ActivePushable && ActivePushable != Pushable)
            ActivePushable->StopPush();

        if (!Pushable->StartPush(ToBoxDir, GetActorLocation()))
            return;

        ActivePushable = Pushable;
        SetSprinting(false);
    }

    if (CatMove)
        CatMove->MaxWalkSpeed = CatMove->DefaultWalkSpeed * ActivePushable->PushSpeedMultiplier;

    ActivePushable->TickPush(GetActorLocation(), DeltaTime, bDebugPushTrace);
}

void ACatCharacter::SetSprinting(bool bSprint)
{
    UCatCharacterMovementComponent* CatMove =
        Cast<UCatCharacterMovementComponent>(GetCharacterMovement());
    if (!CatMove) return;

    if (bSprint && ActivePushable)
        return;

    CatMove->bIsSprinting = bSprint;
    CatMove->MaxWalkSpeed = bSprint ? CatMove->SprintSpeed : CatMove->DefaultWalkSpeed;
}

void ACatCharacter::TryJump()
{
    if (bIsJumping || bIsOnCooldown || bIsEdgeDropping || bIsFreeFalling)
        return;

    if (GetCharacterMovement()->IsFalling())
        return;

    // Block jump if ceiling is too low
    FCollisionQueryParams CeilingParams;
    CeilingParams.AddIgnoredActor(this);
    FHitResult CeilingHit;
    bool bCeilingBlocked = GetWorld()->SweepSingleByChannel(
        CeilingHit,
        GetActorLocation(),
        GetActorLocation() + FVector(0, 0, JumpCeilingCheckHeight),
        FQuat::Identity,
        ECC_WorldStatic,
        FCollisionShape::MakeSphere(GetCapsuleComponent()->GetScaledCapsuleRadius()),
        CeilingParams
    );

    if (bCeilingBlocked)
        return;

    if (CanVaultWithMinHeight())
    {
        ExecuteVault();
        return;
    }

    float Speed = GetVelocity().Size2D();
    float TakeoffDelay = 0.f;

    if (Speed < 50.f)
    {
        ActiveJumpMontage = JumpMontage_Idle;
        TakeoffDelay = JumpTakeoffDelay_Idle;
    }
    else if (Speed < 200.f)
    {
        ActiveJumpMontage = JumpMontage_Walk;
        TakeoffDelay = JumpTakeoffDelay_Walk;
    }
    else
    {
        ActiveJumpMontage = JumpMontage_Run;
        TakeoffDelay = JumpTakeoffDelay_Run;
    }

    if (!ActiveJumpMontage)
        ActiveJumpMontage = JumpMontage_Idle;

    if (!ActiveJumpMontage)
        return;

    int32 LandSectionIdx = ActiveJumpMontage->GetSectionIndex(FName("JumpLand"));
    ActiveLandGatePos = LandSectionIdx != INDEX_NONE
        ? ActiveJumpMontage->GetAnimCompositeSection(LandSectionIdx).GetTime()
        : 0.f;

    bIsJumping = true;
    AirborneTicks = 0;
    bPassedLandGate = false;

    if (UCatCharacterMovementComponent* CatMove =
        Cast<UCatCharacterMovementComponent>(GetCharacterMovement()))
    {
        CatMove->NotifyJumpStart();
    }

    PlayAnimMontage(ActiveJumpMontage, 1.0f, NAME_None);
    GetWorldTimerManager().SetTimer(
        JumpTakeoffTimerHandle,
        this,
        &ACatCharacter::ExecuteJumpTakeoff,
        TakeoffDelay,
        false
    );
}

void ACatCharacter::TryDescend()
{
    if (bIsJumping || bIsOnCooldown || bIsEdgeDropping || bIsFreeFalling || bIsCrawling)
        return;

    ExecuteEdgeDrop(false);
}

void ACatCharacter::ExecuteJumpTakeoff()
{
    float JumpZVelocity = GetCharacterMovement()->JumpZVelocity;
    LaunchCharacter(FVector(0.f, 0.f, JumpZVelocity), false, true);
}

void ACatCharacter::ExecuteEdgeDrop(bool bSkipExitAnim)
{
    FVector Forward = GetActorForwardVector();
    FVector DropStart = GetActorLocation() + Forward * BodyHalfLength * 1.75f;
    FVector DropEnd = DropStart - FVector(0, 0, MinEdgeDropHeight + 200.f);
    FHitResult DropHit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bFoundGround = GetWorld()->LineTraceSingleByChannel(
        DropHit, DropStart, DropEnd, ECC_WorldStatic, Params);

    if (bFoundGround && (DropStart.Z - DropHit.ImpactPoint.Z) < MinEdgeDropHeight)
        return;

    // Ignore the floor mesh so the forward sweep doesn't catch the ledge face
    FHitResult FloorHit;
    FVector FloorStart = GetActorLocation();
    FVector FloorEnd = FloorStart - FVector(0, 0, 100.f);
    if (GetWorld()->LineTraceSingleByChannel(FloorHit, FloorStart, FloorEnd, ECC_WorldStatic, Params))
        Params.AddIgnoredActor(FloorHit.GetActor());

    // Forward clearance check to block drop if the cat can't exit into open space
    {
        UCapsuleComponent* Cap = GetCapsuleComponent();

        FVector SweepOrigin = GetActorLocation()
            - FVector(0, 0, Cap->GetScaledCapsuleHalfHeight() * 0.5f)
            + Forward * BodyHalfLength;
        FVector SweepEnd = SweepOrigin + Forward * EdgeDropForwardDistance;
        const float ClearSweepRadius = Cap->GetScaledCapsuleRadius() * 0.5f;
        FCollisionShape ClearShape = FCollisionShape::MakeSphere(ClearSweepRadius);

        FHitResult ForwardHit;
        bool bForwardBlocked = GetWorld()->SweepSingleByChannel(
            ForwardHit,
            SweepOrigin,
            SweepEnd,
            FQuat::Identity,
            ECC_WorldStatic,
            ClearShape,
            Params
        );

        if (bDebugEdgeDrop)
        {
            DrawDebugLine(GetWorld(), SweepOrigin, SweepEnd,
                bForwardBlocked ? FColor::Red : FColor::Green, false, 3.f, 0, 2.f);
            DrawDebugSphere(GetWorld(), SweepOrigin, SweepRadius,
                8, FColor::Yellow, false, 3.f);
            DrawDebugSphere(GetWorld(), SweepEnd, SweepRadius,
                8, bForwardBlocked ? FColor::Red : FColor::Green, false, 3.f);

            if (bForwardBlocked)
                DrawDebugSphere(GetWorld(), ForwardHit.ImpactPoint, SweepRadius, 8, FColor::Red, false, 3.f);

            GEngine->AddOnScreenDebugMessage(-1, 3.f, bForwardBlocked ? FColor::Red : FColor::Green,
                FString::Printf(TEXT("EdgeDrop forward check: %s  HitActor: %s  SweepOriginZ: %.1f  SweepRadius: %.1f"),
                    bForwardBlocked ? TEXT("BLOCKED") : TEXT("CLEAR"),
                    bForwardBlocked && ForwardHit.GetActor() ? *ForwardHit.GetActor()->GetName() : TEXT("none"),
                    SweepOrigin.Z,
                    SweepRadius));
        }

        if (bForwardBlocked)
            return;
    }

    bIsEdgeDropping = !bSkipExitAnim;
    bIsFreeFalling = bSkipExitAnim;
    bPassedFallLandGate = false;

    EdgeDropOrigin = GetActorLocation();
    EdgeDropTarget = GetActorLocation() + Forward * EdgeDropForwardDistance;
    EdgeDropTarget.Z = GetActorLocation().Z;
    EdgeDropAccumulator = 0.f;

    if (bSkipExitAnim)
    {
        GetCharacterMovement()->SetMovementMode(MOVE_Falling);
        if (FallingMontage)
            PlayAnimMontage(FallingMontage, 1.0f);
    }
    else
    {
        GetCharacterMovement()->DisableMovement();
        if (EdgeExitMontage)
            PlayAnimMontage(EdgeExitMontage, 1.0f);
    }
}

void ACatCharacter::Landed(const FHitResult& Hit)
{
    float ImpactVelZ = GetVelocity().Z;
    
    Super::Landed(Hit);

    RespawnComponent->NotifyLanded(Hit, ImpactVelZ);
    
    bHasPotentialVault = false;

    bool bShouldKillVelocity = GetVelocity().Z < LandingVelocityKillThreshold;

    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    FAnimMontageInstance* MontageInst = (AnimInst && ActiveJumpMontage)
        ? AnimInst->GetInstanceForMontage(ActiveJumpMontage)
        : nullptr;

    if (bIsFreeFalling)
    {
        ResetEdgeDropState();
        if (bShouldKillVelocity)
        {
            GetCharacterMovement()->StopMovementImmediately();
            if (FallLandMontage)
                PlayAnimMontage(FallLandMontage, 1.0f);
        }
        return;
    }

    if (bIsJumping && ActiveJumpMontage)
    {
        if (bShouldKillVelocity)
        {
            ResetCatJumpState();
            GetCharacterMovement()->StopMovementImmediately();
            if (FallLandMontage)
                PlayAnimMontage(FallLandMontage, 1.0f);
            return;
        }

        if (AnimInst && MontageInst)
        {
            bPassedLandGate = true;
            MontageInst->SetPosition(ActiveLandGatePos);
            MontageInst->SetPlayRate(1.f);
            return;
        }

        ResetCatJumpState();
        return;
    }

    if (bShouldKillVelocity)
        GetCharacterMovement()->StopMovementImmediately();
}

void ACatCharacter::UpdateVaultDetection()
{
    bCanVault = false;

    if (bIsVaulting)
        return;

    FVector Start = GetActorLocation();
    FVector Forward = GetActorForwardVector();
    FVector WallTraceEnd = Start + Forward * WallTraceDistance;

    FHitResult WallHit;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(SweepRadius);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHitWall = GetWorld()->SweepSingleByChannel(
        WallHit, Start, WallTraceEnd, FQuat::Identity,
        ECC_WorldStatic, Sphere, Params
    );

    if (bDebugVaultDetection)
        DrawDebugLine(GetWorld(), Start, WallTraceEnd, bHitWall ? FColor::Green : FColor::Red, false, 0.2f, 0, 1.f);

    if (!bHitWall)
    {
        bHasPotentialVault = false;
        return;
    }

    const bool bIsFalling = GetCharacterMovement()->IsFalling();
    const float EffectiveMinHeight = bIsFalling ? -LedgeGrabMaxHeight : MinVaultHeight;
    const float StepSize       = SweepRadius * 0.25f;
    const float ScanBottom     = bIsFalling ? Start.Z - LedgeGrabMaxHeight : Start.Z;
    const float ScanTop        = Start.Z + MaxClimbHeight + JumpReachHeight;
    const float CatStandHeight = VaultCeilingClearance;

    struct FStepResult { float Z; bool bHit; };
    TArray<FStepResult> Steps;

    for (float TestZ = ScanBottom; TestZ <= ScanTop; TestZ += StepSize)
    {
        FVector TraceStart = FVector(Start.X, Start.Y, TestZ);
        FVector TraceEnd   = TraceStart + Forward * (WallTraceDistance + SweepRadius);

        FHitResult StepHit;
        bool bHit = GetWorld()->SweepSingleByChannel(
            StepHit, TraceStart, TraceEnd, FQuat::Identity,
            ECC_WorldStatic, Sphere, Params
        );

        Steps.Add({ TestZ, bHit });

        if (bDebugVaultDetection)
            DrawDebugLine(GetWorld(), TraceStart, TraceEnd,
                bHit ? FColor::Red : FColor::Green, false, 0.2f, 0, 1.f);
    }

    float LedgeZ = -BIG_NUMBER;

    for (int32 i = 0; i < Steps.Num(); i++)
    {
        if (Steps[i].Z < Start.Z + EffectiveMinHeight)
            continue;

        if (!Steps[i].bHit)
            continue;

        if (i + 1 >= Steps.Num())
            break;

        if (Steps[i + 1].bHit)
            continue;

        bool bCeilingTooLow = false;
        for (int32 j = i + 1; j < Steps.Num(); j++)
        {
            if (Steps[j].Z > Steps[i].Z + CatStandHeight)
                break;

            if (Steps[j].bHit)
            {
                bCeilingTooLow = true;
                break;
            }
        }

        if (bCeilingTooLow)
            continue;

        LedgeZ = Steps[i].Z;
        break;
    }
    
    if (LedgeZ <= -BIG_NUMBER)
    {
        for (int32 i = Steps.Num() - 1; i >= 0; i--)
        {
            if (Steps[i].Z < Start.Z + EffectiveMinHeight)
                break;

            if (Steps[i].bHit)
            {
                bool bClearAbove = true;
                for (int32 j = i + 1; j < Steps.Num() && Steps[j].Z <= Steps[i].Z + CatStandHeight; j++)
                {
                    if (Steps[j].bHit) { bClearAbove = false; break; }
                }
                if (bClearAbove)
                    LedgeZ = Steps[i].Z;
                break;
            }
        }
    }

    if (bDebugVaultDetection)
    {
        GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::White,
            FString::Printf(TEXT("LedgeZ: %.1f  StartZ: %.1f  ClimbH: %.1f  MinVault: %.1f  MaxVault: %.1f"),
            LedgeZ, Start.Z, LedgeZ - Start.Z, MinVaultHeight, MaxClimbHeight + JumpReachHeight));
    }

    if (LedgeZ <= -BIG_NUMBER)
    {
        bHasPotentialVault = false;
        return;
    }

    // Refine LedgeZ upward to find clean clearance on sloped surfaces
    const float RefinementRange = SweepRadius * 2.f;
    for (float RefineZ = LedgeZ; RefineZ <= LedgeZ + RefinementRange; RefineZ += StepSize)
    {
        FVector TraceStart = FVector(Start.X, Start.Y, RefineZ);
        FVector TraceEnd   = TraceStart + Forward * (WallTraceDistance + SweepRadius);

        FHitResult RefineHit;
        bool bHit = GetWorld()->SweepSingleByChannel(
            RefineHit, TraceStart, TraceEnd, FQuat::Identity,
            ECC_WorldStatic, Sphere, Params
        );

        if (!bHit)
        {
            LedgeZ = RefineZ;
            break;
        }
    }

    float ClimbHeight = LedgeZ - Start.Z;

    const float EffectiveMinClimb = bIsFalling ? -LedgeGrabMaxHeight : MinVaultHeight;
    if (ClimbHeight < EffectiveMinClimb || ClimbHeight > MaxClimbHeight + JumpReachHeight)
    {
        bHasPotentialVault = false;
        return;
    }

    FVector LedgePoint = FVector(WallHit.ImpactPoint.X, WallHit.ImpactPoint.Y, LedgeZ);

    FVector ClearStart = LedgePoint + FVector(0, 0, SweepRadius * 2.f);
    FVector ClearEnd   = ClearStart + Forward * WallTraceDistance;

    FHitResult ClearHit;
    bool bObstructed = GetWorld()->SweepSingleByChannel(
        ClearHit, ClearStart, ClearEnd, FQuat::Identity,
        ECC_WorldStatic, Sphere, Params
    );

    if (bDebugVaultDetection)
    {
        GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Cyan,
            FString::Printf(TEXT("Obstructed: %d  ClearStart Z: %.1f"), bObstructed ? 1 : 0, ClearStart.Z));
        DrawDebugLine(GetWorld(), ClearStart, ClearEnd,
            bObstructed ? FColor::Red : FColor::Green, false, 0.2f, 0, 1.f);
        DrawDebugSphere(GetWorld(), LedgePoint, 8.f, 8,
            bObstructed ? FColor::Red : FColor::Yellow, false, 0.2f);
    }

    if (bObstructed)
    {
        bHasPotentialVault = false;
        return;
    }

    PotentialVaultTarget = LedgePoint;
    PotentialWallHitLocation = WallHit.ImpactPoint;
    PotentialWallHitNormal = WallHit.ImpactNormal;
    bHasPotentialVault = true;

    if (ClimbHeight <= MaxClimbHeight && ClimbHeight >= 0.f)
    {
        WallHitLocation = WallHit.ImpactPoint;
        WallHitNormal = WallHit.ImpactNormal;
        VaultTargetLocation = LedgePoint;
        bCanVault = true;
    }

    if (bDebugVaultDetection)
        DrawDebugSphere(GetWorld(), LedgePoint, 10.f, 8, FColor::Green, false, 0.2f);
}

void ACatCharacter::ExecuteVault()
{
    if (bDebugVaultSlowMotion)
        GetWorld()->GetWorldSettings()->TimeDilation = 0.1f;
    
    ResetCatJumpState();
    bHasPotentialVault = false;

    bIsOnCooldown = true;
    CooldownTimer = VaultCooldown;

    float ClimbHeight = VaultTargetLocation.Z - WallHitLocation.Z;
    ClimbHeight = FMath::Max(ClimbHeight, 10.f);

    VaultOriginLocation = GetActorLocation();
    
    FVector WallNudge = FVector(WallHitNormal.X, WallHitNormal.Y, 0.f).GetSafeNormal();

    VaultIntermediateLocation = FVector(
        WallHitLocation.X, WallHitLocation.Y,
        VaultTargetLocation.Z + VaultLandingZOffset
    ) + WallNudge * VaultIntermediateBackNudge;

    FVector OntoLedge = FVector(-WallHitNormal.X, -WallHitNormal.Y, 0.f).GetSafeNormal();
    VaultFinalLocation = VaultIntermediateLocation + OntoLedge * VaultPhase2ForwardNudge;

    VaultPhase1Duration = FMath::Max(ClimbHeight * VaultPhase1DurationMultiplier, MinVaultPhase1Duration);
    bIsVaulting = true;

    OnVaultConfirmed(VaultTargetLocation, ClimbHeight);
}

void ACatCharacter::ExecuteLedgeGrab()
{
    if (bDebugVaultSlowMotion)
        GetWorld()->GetWorldSettings()->TimeDilation = 0.1f;
    
    ResetCatJumpState();
    bHasPotentialVault = false;

    bIsOnCooldown = true;
    CooldownTimer = VaultCooldown;

    float ClimbHeight = VaultTargetLocation.Z - GetActorLocation().Z;
    ClimbHeight = FMath::Max(ClimbHeight, 10.f);

    VaultOriginLocation = GetActorLocation();
    VaultIntermediateLocation = FVector(WallHitLocation.X, WallHitLocation.Y, VaultTargetLocation.Z + VaultLandingZOffset);

    FVector OntoLedge = FVector(-WallHitNormal.X, -WallHitNormal.Y, 0.f).GetSafeNormal();
    VaultFinalLocation = VaultIntermediateLocation + OntoLedge * VaultPhase2ForwardNudge;

    VaultPhase1Duration = FMath::Max(ClimbHeight * VaultPhase1DurationMultiplier, MinVaultPhase1Duration);
    bIsVaulting = true;

    if (LedgeGrabMontage)
        PlayAnimMontage(LedgeGrabMontage, 1.0f);

    OnVaultConfirmed(VaultTargetLocation, ClimbHeight);
}

void ACatCharacter::StartCrawl(FVector Entry, FVector Exit)
{
    CrawlEntry = Entry;
    CrawlExit = Exit;
    CrawlElapsed = 0.f;
    bIsCrawling = true;
    GetCharacterMovement()->DisableMovement();

    FVector Direction = (Exit - Entry).GetSafeNormal();
    SetActorRotation(Direction.Rotation());

    SetActorLocation(Entry);
}

void ACatCharacter::TryCrawl(ACrawlSpace* CrawlSpace)
{
    if (!CrawlSpace || bIsCrawling)
        return;

    FVector Entry, Exit;
    if (!CrawlSpace->GetCrawlPoints(GetActorLocation(), Entry, Exit))
        return;

    CrawlEntry = Entry;
    CrawlExit = Exit;
    CrawlElapsed = 0.f;
    CrawlDelayElapsed = 0.f;
    bCrawlExitPending = false;
    bIsCrawling = true;

    GetCharacterMovement()->DisableMovement();
    DetachCamera();
    
    SetActorLocation(Entry);
    SetActorRotation((Exit - Entry).GetSafeNormal().Rotation());

    if (CrawlMontage)
        PlayAnimMontage(CrawlMontage, 1.0f);
}

void ACatCharacter::TryChuteCrawl(AChute* Chute)
{
    if (!Chute || bIsChuteCrawling || bIsCrawling)
        return;

    FVector Entry, Exit, EntryForward;
    if (!Chute->GetChutePoints(GetActorLocation(), Entry, Exit, EntryForward))
        return;

    ChuteEntry = Entry;
    ChuteExit = Exit;
    ChuteEntryForward = EntryForward;
    ChuteEntryLerpDistance = Chute->EntryLerpDistance;
    ChuteEntryLerpDuration = Chute->EntryLerpDuration;
    ChuteExitLerpDistance = Chute->ExitLerpDistance;
    ChuteExitLerpDuration = Chute->ExitLerpDuration;
    ChuteElapsed = 0.f;
    bChuteTeleportDone = false;
    bIsChuteCrawling = true;

    // Teleport location is exit point pulled back by ExitLerpDistance along entry forward
    // to simulate the cat "exiting" from behind the point, moving into the point
    ChuteTeleportLocation = ChuteExit - ChuteEntryForward * ChuteExitLerpDistance;

    GetCharacterMovement()->DisableMovement();
    DetachCamera();
    
    SetActorLocation(Entry);
    SetActorRotation(EntryForward.Rotation());

    if (ChuteMontage)
        PlayAnimMontage(ChuteMontage, 1.0f);
}

void ACatCharacter::TickChuteCrawl(float DeltaTime)
{
    if (!bIsChuteCrawling)
        return;

    ChuteElapsed += DeltaTime;

    if (!bChuteTeleportDone)
    {
        // Crawl forward from entry by EntryLerpDistance
        float Alpha = FMath::Clamp(ChuteElapsed / ChuteEntryLerpDuration, 0.f, 1.f);
        FVector EntryTarget = ChuteEntry + ChuteEntryForward * ChuteEntryLerpDistance;
        SetActorLocation(FMath::Lerp(ChuteEntry, EntryTarget, Alpha));

        if (Alpha >= 1.f)
        {
            bChuteTeleportDone = true;
            ChuteElapsed = 0.f;
            SetActorLocation(ChuteTeleportLocation);
            SetActorRotation(ChuteEntryForward.Rotation());
        }
    }
    else
    {
        // Crawl forward from teleport location to exit
        float Alpha = FMath::Clamp(ChuteElapsed / ChuteExitLerpDuration, 0.f, 1.f);
        SetActorLocation(FMath::Lerp(ChuteTeleportLocation, ChuteExit, Alpha));

        if (Alpha >= 1.f)
        {
            bIsChuteCrawling = false;
            GetCharacterMovement()->SetMovementMode(MOVE_Walking);
            ReattachCamera();
            OnChuteCrawlComplete();
        }
    }
}

void ACatCharacter::DetachCamera()
{
    USpringArmComponent* Boom = FindComponentByClass<USpringArmComponent>();
    if (!Boom) return;

    CameraAnchor = NewObject<USceneComponent>(this, TEXT("CameraAnchor"));
    CameraAnchor->RegisterComponent();
    CameraAnchor->SetWorldLocationAndRotation(
        Boom->GetComponentLocation(),
        Boom->GetComponentRotation()
    );

    Boom->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
    Boom->AttachToComponent(CameraAnchor, FAttachmentTransformRules::KeepWorldTransform);
}

void ACatCharacter::ReattachCamera()
{
    USpringArmComponent* Boom = FindComponentByClass<USpringArmComponent>();
    if (!Boom) return;

    Boom->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    Boom->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
    Boom->TargetArmLength = 200.f;

    if (CameraAnchor)
    {
        CameraAnchor->DestroyComponent();
        CameraAnchor = nullptr;
    }
}

float ACatCharacter::GetTurnRate() const
{
    return SmoothedTurnRate;
}

void ACatCharacter::NotifyEnteredWater()
{
    if (RespawnComponent)
        RespawnComponent->NotifyEnteredWater();
}

void ACatCharacter::CycleHat()
{
    UCatGameInstance* GI = Cast<UCatGameInstance>(GetGameInstance());
    if (!GI) return;

    UCatHatModifier* NextHat = GI->GetNextUnlockedHat(AppearanceComponent->Hat);
    AppearanceComponent->SetHat(NextHat);
    GI->SetEquippedHat(NextHat);
}

void ACatCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (InteractAction)
            EIC->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ACatCharacter::PerformInteract);
    }
}

void ACatCharacter::PerformInteract()
{
    FVector Start = GetActorLocation();
    FVector End = Start + GetActorForwardVector() * 50.f;

    TArray<FHitResult> Hits;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    GetWorld()->SweepMultiByChannel(
        Hits, Start, End, FQuat::Identity,
        ECC_Visibility, FCollisionShape::MakeSphere(25.f), Params);

    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan,
        FString::Printf(TEXT("Interact hits: %d"), Hits.Num()));

    for (const FHitResult& Hit : Hits)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan,
            FString::Printf(TEXT("  Hit: %s"), Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("null")));
    }

    AInteractableBase* Target = nullptr;
    for (const FHitResult& Hit : Hits)
    {
        AInteractableBase* Candidate = Cast<AInteractableBase>(Hit.GetActor());
        if (Candidate && Candidate->CanInteract(this))
        {
            Target = Candidate;
            break;
        }
    }

    if (!Target)
        return;

    if (ACrawlSpace* CrawlSpace = Cast<ACrawlSpace>(Target))
    {
        TryCrawl(CrawlSpace);
    }
    else if (AItemPickup* Pickup = Cast<AItemPickup>(Target))
    {
        Pickup->FinishPickup();
    }
    else if (AChute* Chute = Cast<AChute>(Target))
    {
        TryChuteCrawl(Chute);
    }
    else
    {
        IInteractable::Execute_Interact(Target, this);
    }
}