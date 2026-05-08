#include "CatAppearanceComponent.h"

#include "CatAppearance/CatHatModifier.h"
#include "CatAppearance/CatAppearanceModifier.h"
#include "CatAppearance/CatEarTypeModifier.h"
#include "CatAppearance/CatBodyMassModifier.h"
#include "CatAppearance/CatEyeColorModifier.h"
#include "CatAppearance/CatFaceTypeModifier.h"
#include "CatAppearance/CatEyesTypeModifier.h"
#include "CatAppearance/CatHeadTypeModifier.h"
#include "CatAppearance/CatTailTypeModifier.h"
#include "CatAppearance/CatPostureModifier.h"
#include "CatAppearance/CatFurModifier.h"

UCatAppearanceComponent::UCatAppearanceComponent()
{
}

void UCatAppearanceComponent::ApplyToMesh(USkeletalMeshComponent* Mesh) const
{
    if (!Mesh) return;

    for (const UCatAppearanceModifier* Modifier : TArray<const UCatAppearanceModifier*>{
        EarType.Get(), BodyMass.Get(), FaceType.Get(), EyesType.Get(),
        HeadType.Get(), TailType.Get(), Posture.Get() })
    {
        if (!Modifier) continue;
        for (const FName& MorphName : Modifier->OwnedMorphTargets)
            Mesh->SetMorphTarget(MorphName, 0.f);
    }

    ApplyModifier(Mesh, EarType);
    ApplyModifier(Mesh, BodyMass);
    ApplyModifier(Mesh, FaceType);
    ApplyModifier(Mesh, EyesType);
    ApplyModifier(Mesh, HeadType);
    ApplyModifier(Mesh, TailType);
    ApplyModifier(Mesh, Posture);

    ApplyFurSet(Mesh);
    ApplyEyeColor(Mesh);

    ApplyHat(Mesh);
}

void UCatAppearanceComponent::ApplyHat(USkeletalMeshComponent* Mesh) const
{
    // Clear any previously spawned hat by tag
    AActor* Owner = GetOwner();
    if (!Owner) return;

    TArray<UActorComponent*> Existing = Owner->GetComponentsByTag(UStaticMeshComponent::StaticClass(), FName("CatHat"));
    for (UActorComponent* Comp : Existing)
        Comp->DestroyComponent();

    ActiveHatMesh = nullptr;

    if (!Hat || !Hat->HatMesh || !Mesh)
        return;

    UStaticMeshComponent* HatComp = NewObject<UStaticMeshComponent>(Owner);
    HatComp->RegisterComponent();
    HatComp->ComponentTags.Add(FName("CatHat"));
    HatComp->SetStaticMesh(Hat->HatMesh);
    HatComp->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Hat->AttachSocket);
    HatComp->SetRelativeLocationAndRotation(Hat->RelativeOffset, Hat->RelativeRotation);
    HatComp->SetRelativeScale3D(Hat->RelativeScale);
    HatComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ActiveHatMesh = HatComp;
}

void UCatAppearanceComponent::SetHat(UCatHatModifier* NewHat)
{
    Hat = NewHat;

    USkeletalMeshComponent* Mesh = GetOwner()
        ? GetOwner()->FindComponentByClass<USkeletalMeshComponent>()
        : nullptr;

    if (Mesh)
        ApplyHat(Mesh);
}

void UCatAppearanceComponent::ApplyModifier(USkeletalMeshComponent* Mesh, const UCatAppearanceModifier* Modifier) const
{
    if (!Modifier) return;
    for (const auto& Pair : Modifier->Weights)
    {
        float Current = Mesh->GetMorphTarget(Pair.Key);
        float Combined = FMath::Clamp(Current + Pair.Value, -1.f, 1.f);
        Mesh->SetMorphTarget(Pair.Key, Combined);
    }
}

void UCatAppearanceComponent::ApplyFurSet(USkeletalMeshComponent* Mesh) const
{
    if (!FurVariant) return;

    // For convenience since fur MATs come in pairs
    // A hassle otherwise to micromanage and assign 2 at a time lol

    const int32 FurSlotIndex  = Mesh->GetMaterialIndex(TEXT("Cat_Fur"));
    const int32 BodySlotIndex = Mesh->GetMaterialIndex(TEXT("Cat_Body"));

    if (FurSlotIndex  != INDEX_NONE && FurVariant->FurMaterial)
        Mesh->SetMaterial(FurSlotIndex,  FurVariant->FurMaterial);

    if (BodySlotIndex != INDEX_NONE && FurVariant->BodyMaterial)
        Mesh->SetMaterial(BodySlotIndex, FurVariant->BodyMaterial);
}

void UCatAppearanceComponent::ApplyEyeColor(USkeletalMeshComponent* Mesh) const
{
    if (!EyeColor || !EyeColor->EyesMaterial) return;

    const int32 SlotIndex = Mesh->GetMaterialIndex(TEXT("Eyes"));
    if (SlotIndex != INDEX_NONE)
        Mesh->SetMaterial(SlotIndex, EyeColor->EyesMaterial);
}