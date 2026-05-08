#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PushableComponent.h"
#include "CatRespawnComponent.h"
#include "CatAppearanceComponent.h"
#include "CrawlSpace.h"
#include "Chute.h"
#include "ItemPickup.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "CatCharacter.generated.h"

class UInputAction;

UCLASS()
class CATGAME_API ACatCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ACatCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void TryDescend();

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> InteractAction;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetSprinting(bool bSprint);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    float GetTurnRate() const;

    UFUNCTION(BlueprintCallable, Category = "Respawn")
    void NotifyEnteredWater();

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void CycleHat();

protected:
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void Landed(const FHitResult& Hit) override;
    virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

    void PerformInteract();

    // Vault
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vault")
    float WallTraceDistance = 15.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vault")
    float SweepRadius = 20.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vault")
    float MaxClimbHeight = 120.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vault")
    float MinVaultHeight = 40.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vault")
    float MinAirborneVaultHeight = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vault")
    float LedgeGrabMaxHeight = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vault")
    int32 MinAirborneTicks = 10;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vault")
    float ClearanceCheckHeight = 60.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vault")
    float VaultCooldown = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vault")
    float JumpReachHeight = 200.f;

    UPROPERTY(EditAnywhere, Category = "Vault")
    float VaultPhase1DurationMultiplier = 0.004f;

    UPROPERTY(EditAnywhere, Category = "Vault")
    float MinVaultPhase1Duration = 0.3f;

    UPROPERTY(EditAnywhere, Category = "Vault")
    float VaultPhase1HoldDuration = 0.05f;
    
    UPROPERTY(EditAnywhere, Category = "Vault")
    float VaultIntermediateBackNudge = 5.f;
    
    UPROPERTY(EditAnywhere, Category = "Vault")
    float VaultPhase2Duration = 0.25f;

    UPROPERTY(EditAnywhere, Category = "Vault")
    float VaultPhase2ForwardNudge = 30.f;

    UPROPERTY(EditAnywhere, Category = "Vault")
    float VaultLandingZOffset = 50.f;

    UPROPERTY(EditAnywhere, Category = "Vault")
    float MidAirVaultMinVelZ = -250.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vault")
    float VaultGapThreshold = 30.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vault")
    float VaultCeilingClearance = 60.f;

    UPROPERTY(EditAnywhere, Category = "Vault")
    TObjectPtr<UAnimMontage> LedgeGrabMontage;

    UPROPERTY(BlueprintReadOnly, Category = "Vault")
    bool bCanVault = false;

    UPROPERTY(BlueprintReadOnly, Category = "Vault")
    FVector VaultTargetLocation = FVector::ZeroVector;

    UFUNCTION(BlueprintImplementableEvent, Category = "Vault")
    void OnVaultConfirmed(FVector TargetLocation, float ClimbHeight);

    UFUNCTION(BlueprintImplementableEvent, Category = "Vault")
    void OnVaultMovementComplete();

    // Edge drop
    UPROPERTY(EditAnywhere, Category = "EdgeDrop")
    TObjectPtr<UAnimMontage> EdgeExitMontage;

    UPROPERTY(EditAnywhere, Category = "EdgeDrop")
    TObjectPtr<UAnimMontage> FallingMontage;

    UPROPERTY(EditAnywhere, Category = "EdgeDrop")
    TObjectPtr<UAnimMontage> FallLandMontage;

    UPROPERTY(EditAnywhere, Category = "EdgeDrop")
    float MinEdgeDropHeight = 80.f;

    UPROPERTY(EditAnywhere, Category = "EdgeDrop")
    float FallLandDetectionDistance = 150.f;

    UPROPERTY(EditAnywhere, Category = "EdgeDrop")
    float EdgeDropForwardDistance = 80.f;

    UPROPERTY(EditAnywhere, Category = "EdgeDrop")
    float EdgeDropForwardDuration = 0.4f;

    UPROPERTY(EditAnywhere, Category = "EdgeDrop")
    float EdgeDropExitDownVelocity = 300.f;

    UPROPERTY(EditAnywhere, Category = "EdgeDrop")
    float EdgeDropExitForwardVelocity = 150.f;

    // Crawl
    UFUNCTION(BlueprintCallable, Category = "Crawl")
    void StartCrawl(FVector Entry, FVector Exit);

    UFUNCTION(BlueprintCallable, Category = "Crawl")
    void TryCrawl(ACrawlSpace* CrawlSpace);

    UFUNCTION(BlueprintImplementableEvent, Category = "Crawl")
    void OnCrawlComplete();

    UPROPERTY(BlueprintReadOnly, Category = "Crawl")
    bool bIsCrawling = false;

    UPROPERTY(EditAnywhere, Category = "Crawl")
    float CrawlDuration = 2.0f;
    
    UPROPERTY(EditAnywhere, Category = "Crawl")
    float CrawlEntryDelay = 0.1f;

    UPROPERTY(EditAnywhere, Category = "Crawl")
    float CrawlExitDelay = 0.1f;

    UPROPERTY(EditAnywhere, Category = "Crawl")
    TObjectPtr<UAnimMontage> CrawlMontage;


    //
    UFUNCTION(BlueprintCallable, Category = "Chute")
    void TryChuteCrawl(AChute* Chute);

    UFUNCTION(BlueprintImplementableEvent, Category = "Chute")
    void OnChuteCrawlComplete();

    UPROPERTY(BlueprintReadOnly, Category = "Chute")
    bool bIsChuteCrawling = false;

    UPROPERTY(EditAnywhere, Category = "Chute")
    TObjectPtr<UAnimMontage> ChuteMontage;

    // Jump
    UFUNCTION(BlueprintCallable, Category = "Jump")
    void TryJump();

    UPROPERTY(EditAnywhere, Category = "Jump")
    TObjectPtr<UAnimMontage> JumpMontage_Idle;

    UPROPERTY(EditAnywhere, Category = "Jump")
    TObjectPtr<UAnimMontage> JumpMontage_Walk;

    UPROPERTY(EditAnywhere, Category = "Jump")
    TObjectPtr<UAnimMontage> JumpMontage_Run;

    UPROPERTY(BlueprintReadOnly, Category = "Jump")
    TObjectPtr<UAnimMontage> ActiveJumpMontage;

    UPROPERTY(EditAnywhere, Category = "Jump")
    float JumpTakeoffDelay_Idle = 0.2f;

    UPROPERTY(EditAnywhere, Category = "Jump")
    float JumpTakeoffDelay_Walk = 0.15f;

    UPROPERTY(EditAnywhere, Category = "Jump")
    float JumpTakeoffDelay_Run = 0.1f;

    UPROPERTY(EditAnywhere, Category = "Jump")
    float LandingDetectionDistance = 100.f;

    UPROPERTY(EditAnywhere, Category = "Jump")
    float LandingVelocityKillThreshold = -600.f;

    UPROPERTY(EditAnywhere, Category = "Jump")
    float JumpCeilingCheckHeight = 60.f;

    // Movement / slope
    UPROPERTY(EditAnywhere, Category = "Movement")
    FVector MeshOffset = FVector(-15.f, 0.f, -32.5f);
    
    UPROPERTY(EditAnywhere, Category = "Movement")
    float MeshSlopeZCorrectionUp = 25.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float MeshSlopeZCorrectionDown = 10.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float SlopeRotationInterpSpeed = 6.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float SlopeNormalInterpSpeed = 8.f;
    
    UPROPERTY(EditAnywhere, Category = "Movement")
    float BodyHalfLength = 27.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float TurnRateInterpSpeed = 6.f;

    // Push
    UPROPERTY(EditAnywhere, Category = "Movement")
    float PushTraceDistance = 40.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float PushTraceRadius = 20.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float PushCenterBandHalfWidth = 20.f;

    // Debugs
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDebugSlopeTraces = false;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDebugJumpRaycast = false;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDebugVaultDetection = false;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDebugPushTrace = false;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDebugVaultSlowMotion = false;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDebugEdgeDrop = false;
    
    // respawn
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Respawn")
    TObjectPtr<UCatRespawnComponent> RespawnComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Appearance")
    TObjectPtr<UCatAppearanceComponent> AppearanceComponent;
    
private:
    void UpdateVaultDetection();
    void ExecuteVault();
    void ExecuteLedgeGrab();
    void ExecuteEdgeDrop(bool bSkipExitAnim = false);
    void ResetCatJumpState();
    void ResetEdgeDropState();
    void TickVault(float DeltaTime);
    void TickCrawl(float DeltaTime);
    void TickChuteCrawl(float DeltaTime);
    void TickJump(float DeltaTime);
    void TickEdgeDrop(float DeltaTime);
    void TickSlope(float DeltaTime);
    void TickPush(float DeltaTime);
    void ExecuteJumpTakeoff();
    bool CanVaultWithMinHeight() const;

    // Vault
    bool bIsOnCooldown = false;
    float CooldownTimer = 0.f;
    bool bIsVaulting = false;
    float VaultAccumulator = 0.f;
    float VaultPhase1Duration = 0.f;
    FVector VaultOriginLocation = FVector::ZeroVector;
    FVector VaultIntermediateLocation = FVector::ZeroVector;
    FVector VaultFinalLocation = FVector::ZeroVector;
    FVector WallHitLocation = FVector::ZeroVector;
    FVector WallHitNormal = FVector::ZeroVector;

    // precomputed vault target for mid-air vaulting
    bool bHasPotentialVault = false;
    FVector PotentialVaultTarget = FVector::ZeroVector;
    FVector PotentialWallHitLocation = FVector::ZeroVector;
    FVector PotentialWallHitNormal = FVector::ZeroVector;

    // Edge drop
    bool bIsEdgeDropping = false;
    bool bIsFreeFalling = false;
    bool bPassedFallLandGate = false;
    float EdgeDropAccumulator = 0.f;
    FVector EdgeDropOrigin = FVector::ZeroVector;
    FVector EdgeDropTarget = FVector::ZeroVector;

    // Crawl
    FVector CrawlEntry = FVector::ZeroVector;
    FVector CrawlExit = FVector::ZeroVector;
    float CrawlElapsed = 0.f;
    float CrawlDelayElapsed = 0.f;
    bool bCrawlExitPending = false;

    // Chute
    FVector ChuteEntry = FVector::ZeroVector;
    FVector ChuteExit = FVector::ZeroVector;
    FVector ChuteEntryForward = FVector::ZeroVector;
    FVector ChuteTeleportLocation = FVector::ZeroVector;
    float ChuteEntryLerpDistance = 0.f;
    float ChuteEntryLerpDuration = 0.f;
    float ChuteExitLerpDistance = 0.f;
    float ChuteExitLerpDuration = 0.f;
    float ChuteElapsed = 0.f;
    bool bChuteTeleportDone = false;
    
    // Jump
    bool bIsJumping = false;
    int32 AirborneTicks = 0;
    bool bPassedLandGate = false;
    float ActiveLandGatePos = 0.f;
    FTimerHandle JumpTakeoffTimerHandle;
    int32 PushReleaseFrames = 0;
    float FallStartZ = 0.f;

    // Push
    UPROPERTY()
    TObjectPtr<UPushableComponent> ActivePushable;

    // Camera
    void DetachCamera();
    void ReattachCamera();

    UPROPERTY()
    TObjectPtr<USceneComponent> CameraAnchor;

    FVector SmoothedFrontNormal = FVector::UpVector;
    FVector SmoothedRearNormal = FVector::UpVector;

    // Turning
    float TurnRate = 0.f;
    float LastYaw = 0.f;
    float SmoothedTurnRate = 0.f;
};