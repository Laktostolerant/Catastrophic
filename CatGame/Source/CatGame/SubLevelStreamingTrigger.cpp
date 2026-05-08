#include "SubLevelStreamingTrigger.h"
#include "CatGameInstance.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LevelStreaming.h"

ASubLevelStreamingTrigger::ASubLevelStreamingTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    TriggerVolume->SetBoxExtent(FVector(200.f, 200.f, 200.f));
    TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerVolume;
}

void ASubLevelStreamingTrigger::BeginPlay()
{
    Super::BeginPlay();
    TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ASubLevelStreamingTrigger::OnOverlapBegin);
}

void ASubLevelStreamingTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (bTriggered) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!OtherActor || OtherActor != PlayerPawn) return;

    UCatGameInstance* GI = Cast<UCatGameInstance>(GetGameInstance());
    if (!GI) return;

    // Suppress if already loaded
    const bool bAlreadyLoaded = !LevelToLoad.IsNone()
        && GI->GetLoadedSublevels().Contains(LevelToLoad);

    if (bAlreadyLoaded) return;

    bTriggered = true;

    DoUnload();
    DoLoad();
}

void ASubLevelStreamingTrigger::DoLoad()
{
    if (LevelToLoad.IsNone()) return;

    FLatentActionInfo LatentInfo;
    LatentInfo.CallbackTarget = this;
    LatentInfo.ExecutionFunction = FName("OnLevelLoaded");
    LatentInfo.UUID = 2;
    LatentInfo.Linkage = 0;

    UGameplayStatics::LoadStreamLevel(this, LevelToLoad, bMakeVisibleAfterLoad, false, LatentInfo);
}

void ASubLevelStreamingTrigger::DoUnload()
{
    if (LevelToUnload.IsNone()) return;

    FLatentActionInfo LatentInfo;
    LatentInfo.CallbackTarget = this;
    LatentInfo.ExecutionFunction = FName("OnLevelUnloaded");
    LatentInfo.UUID = 1;
    LatentInfo.Linkage = 0;

    UGameplayStatics::UnloadStreamLevel(this, LevelToUnload, LatentInfo, false);
}

void ASubLevelStreamingTrigger::OnLevelLoaded()
{
    UCatGameInstance* GI = Cast<UCatGameInstance>(GetGameInstance());
    if (GI) GI->NotifySubLevelLoaded(LevelToLoad);
}

void ASubLevelStreamingTrigger::OnLevelUnloaded()
{
    UCatGameInstance* GI = Cast<UCatGameInstance>(GetGameInstance());
    if (GI) GI->NotifySubLevelUnloaded(LevelToUnload);
}