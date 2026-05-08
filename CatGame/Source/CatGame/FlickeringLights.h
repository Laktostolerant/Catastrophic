#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/LightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "FlickeringLights.generated.h"

UENUM(BlueprintType)
enum class ELightFlickerMode : uint8
{
    On          UMETA(DisplayName = "On (No Flicker)"),
    Off         UMETA(DisplayName = "Off (No Flicker)"),
    MostlyOff   UMETA(DisplayName = "Mostly Off (Flickers On)"),
    MostlyOn    UMETA(DisplayName = "Mostly On (Flickers Off)"),
    Continuous  UMETA(DisplayName = "Continuous Flicker")
};

UCLASS()
class CATGAME_API AFlickeringLights : public AActor
{
    GENERATED_BODY()

public:
    AFlickeringLights();

    UFUNCTION(BlueprintCallable, Category = "FlickeringLights")
    void SwitchFlickerType(ELightFlickerMode NewFlickerMode);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "FlickeringLights")
    void SetVisibility(bool bVisible);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker")
    float FlickerFrequency = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker")
    float FlickerDuration = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker")
    float FlickerToggleRate = 0.07f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker")
    bool bUniformFlicker = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker")
    ELightFlickerMode FlickerMode = ELightFlickerMode::On;

private:
    TArray<ULightComponent*> ManagedLights;
    TArray<float> OriginalIntensities;
    TArray<bool> bLightCurrentlyOn;
    TArray<FTimerHandle> LightFlickerTimers;

    FTimerHandle FlickerEventTimer;
    FTimerHandle FlickerBurstTimer;
    
    UMaterialInstanceDynamic* LitMat = nullptr;

    float OriginalEmissiveIntensity = 0.0f;

    void ScheduleNextFlickerEvent();
    void StartFlickerBurst();
    void EndFlickerBurst();

    void ToggleLightUniform();
    void ToggleLightIndividual(int32 Index);

    void SetLightOn(int32 Index, bool bOn);
    void EndPlay(EEndPlayReason::Type EndPlayReason);
    void ApplyBaselineState();
};