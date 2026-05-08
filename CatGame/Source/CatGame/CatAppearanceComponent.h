#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "CatAppearanceComponent.generated.h"

class UCatHatModifier;
class UCatEyeColorModifier;
class UCatFurModifier;
class UCatAppearanceModifier;
class UCatPostureModifier;
class UCatTailTypeModifier;
class UCatEyesTypeModifier;
class UCatFaceTypeModifier;
class UCatBodyMassModifier;
class UCatEarTypeModifier;
class UCatHeadTypeModifier;

UCLASS(ClassGroup=Cat, meta=(BlueprintSpawnableComponent))
class CATGAME_API UCatAppearanceComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UCatAppearanceComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TObjectPtr<UCatHatModifier> Hat;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TObjectPtr<UCatFurModifier> FurVariant;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TObjectPtr<UCatEarTypeModifier> EarType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TObjectPtr<UCatBodyMassModifier> BodyMass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TObjectPtr<UCatFaceTypeModifier> FaceType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TObjectPtr<UCatEyesTypeModifier> EyesType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TObjectPtr<UCatEyeColorModifier> EyeColor;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TObjectPtr<UCatHeadTypeModifier> HeadType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TObjectPtr<UCatTailTypeModifier> TailType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TObjectPtr<UCatPostureModifier> Posture;

	void ApplyToMesh(USkeletalMeshComponent* Mesh) const;
	void ApplyHat(USkeletalMeshComponent* Mesh) const;

	UFUNCTION(BlueprintCallable, Category="Appearance")
	void SetHat(UCatHatModifier* NewHat);

private:
	void ApplyModifier(USkeletalMeshComponent* Mesh, const UCatAppearanceModifier* Modifier) const;
	void ApplyFurSet(USkeletalMeshComponent* Mesh) const;
	void ApplyEyeColor(USkeletalMeshComponent* Mesh) const;

	UPROPERTY()
	mutable TObjectPtr<UStaticMeshComponent> ActiveHatMesh;

	TObjectPtr<USkeletalMeshComponent> TargetMesh;
};