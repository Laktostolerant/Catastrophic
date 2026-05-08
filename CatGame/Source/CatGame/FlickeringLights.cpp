#include "FlickeringLights.h"
#include "TimerManager.h"
#include "Engine/World.h"

AFlickeringLights::AFlickeringLights()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AFlickeringLights::SwitchFlickerType(ELightFlickerMode NewFlickerMode)
{
    FlickerMode = NewFlickerMode;
    ApplyBaselineState();

    if (NewFlickerMode == ELightFlickerMode::On || NewFlickerMode == ELightFlickerMode::Off)
    {
        GetWorldTimerManager().ClearTimer(FlickerEventTimer);
        GetWorldTimerManager().ClearTimer(FlickerBurstTimer);
        for (FTimerHandle& Handle : LightFlickerTimers)
        {
            GetWorldTimerManager().ClearTimer(Handle);
        }
    }
    else
    {
        ScheduleNextFlickerEvent();
    }
}

void AFlickeringLights::BeginPlay()
{
    Super::BeginPlay();

    GetComponents<ULightComponent>(ManagedLights);

    for (ULightComponent* Light : ManagedLights)
    {
        OriginalIntensities.Add(Light->Intensity);
        bLightCurrentlyOn.Add(true);
        LightFlickerTimers.Add(FTimerHandle());
    }

    if (FObjectProperty* Prop = CastField<FObjectProperty>(GetClass()->FindPropertyByName(FName("LitMat"))))
    {
        LitMat = Cast<UMaterialInstanceDynamic>(Prop->GetObjectPropertyValue_InContainer(this));
        if (LitMat)
        {
            LitMat->GetScalarParameterValue(FName("Light Intensity"), OriginalEmissiveIntensity);
        }
    }

    ApplyBaselineState();
    ScheduleNextFlickerEvent();
}

void AFlickeringLights::ApplyBaselineState()
{
    for (int32 i = 0; i < ManagedLights.Num(); i++)
    {
        bool bOn = (FlickerMode != ELightFlickerMode::MostlyOff && FlickerMode != ELightFlickerMode::Off);
        SetLightOn(i, bOn);
    }
}

void AFlickeringLights::ScheduleNextFlickerEvent()
{
    if (FlickerMode == ELightFlickerMode::On || FlickerMode == ELightFlickerMode::Off) return;

    float Variance = FlickerFrequency * 0.3f;
    float Delay = FlickerFrequency + FMath::RandRange(-Variance, Variance);
    Delay = FMath::Max(Delay, 0.5f);

    GetWorldTimerManager().SetTimer(
        FlickerEventTimer, this,
        &AFlickeringLights::StartFlickerBurst,
        Delay, false
    );
}

void AFlickeringLights::StartFlickerBurst()
{
    if (ManagedLights.IsEmpty() || LightFlickerTimers.IsEmpty()) return;
    
    if (bUniformFlicker)
    {
        GetWorldTimerManager().SetTimer(
            LightFlickerTimers[0], this,
            &AFlickeringLights::ToggleLightUniform,
            FlickerToggleRate, true
        );
    }
    else
    {
        for (int32 i = 0; i < ManagedLights.Num(); i++)
        {
            float Offset = FMath::RandRange(0.0f, FlickerToggleRate);
            int32 CapturedIndex = i;
            FTimerDelegate Delegate;
            Delegate.BindLambda([this, CapturedIndex]()
            {
                ToggleLightIndividual(CapturedIndex);
            });
            GetWorldTimerManager().SetTimer(
                LightFlickerTimers[i], Delegate,
                FlickerToggleRate + Offset, true
            );
        }
    }

    float ActualDuration = FlickerDuration + FMath::RandRange(-FlickerToggleRate * 2, FlickerToggleRate * 2);

    GetWorldTimerManager().SetTimer(
        FlickerBurstTimer, this,
        &AFlickeringLights::EndFlickerBurst,
        ActualDuration, false
    );
}

void AFlickeringLights::EndFlickerBurst()
{
    for (FTimerHandle& Handle : LightFlickerTimers)
    {
        GetWorldTimerManager().ClearTimer(Handle);
    }

    if (FlickerMode != ELightFlickerMode::Continuous)
    {
        ApplyBaselineState();
    }

    ScheduleNextFlickerEvent();
}

void AFlickeringLights::ToggleLightUniform()
{
    bool bNewState = !bLightCurrentlyOn[0];
    for (int32 i = 0; i < ManagedLights.Num(); i++)
    {
        SetLightOn(i, bNewState);
    }
}

void AFlickeringLights::ToggleLightIndividual(int32 Index)
{
    SetLightOn(Index, !bLightCurrentlyOn[Index]);
}

void AFlickeringLights::SetLightOn(int32 Index, bool bOn)
{
    if (!ManagedLights.IsValidIndex(Index)) return;

    bLightCurrentlyOn[Index] = bOn;
    ManagedLights[Index]->SetIntensity(bOn ? OriginalIntensities[Index] : 0.0f);

    if (LitMat)
    {
        LitMat->SetScalarParameterValue(FName("Light Intensity"), bOn ? OriginalEmissiveIntensity : 0.0f);
    }
}

void AFlickeringLights::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    GetWorldTimerManager().ClearTimer(FlickerEventTimer);
    GetWorldTimerManager().ClearTimer(FlickerBurstTimer);
    for (FTimerHandle& Handle : LightFlickerTimers)
    {
        GetWorldTimerManager().ClearTimer(Handle);
    }
}