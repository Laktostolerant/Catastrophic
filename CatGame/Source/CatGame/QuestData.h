#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemData.h"
#include "CatAppearance/CatHatModifier.h"
#include "QuestData.generated.h"

UENUM(BlueprintType)
enum class EQuestType : uint8
{
    Main    UMETA(DisplayName = "Main Quest"),
    Side    UMETA(DisplayName = "Side Quest")
};

UCLASS(BlueprintType)
class CATGAME_API UQuestData : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    FName QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    EQuestType QuestType = EQuestType::Side;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    TArray<FText> OfferLines;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    TObjectPtr<UItemData> RequiredItem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    int32 RequiredQty = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    TObjectPtr<UItemData> RewardItem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    TObjectPtr<UCatHatModifier> RewardHat;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    TArray<FName> CompletionEffects;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    TArray<FText> CompletionLines;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    int32 RewardQty = 1;

    FText GetSummaryText() const
    {
        FText ObtainLine = RequiredItem
            ? FText::Format(NSLOCTEXT("Quest", "Obtain", "Obtain: {0} {1}"), FText::AsNumber(RequiredQty), RequiredItem->DisplayName)
            : FText::GetEmpty();

        FText RewardLine = RewardItem
            ? FText::Format(NSLOCTEXT("Quest", "Reward", "Reward: +{0} {1}"), FText::AsNumber(RewardQty), RewardItem->DisplayName)
            : FText::GetEmpty();

        FText HatLine = RewardHat
            ? FText::Format(NSLOCTEXT("Quest", "RewardHat", "Obtained: {0}"), RewardHat->DisplayName)
            : FText::GetEmpty();

        TArray<FText> Lines;
        if (!ObtainLine.IsEmpty()) Lines.Add(ObtainLine);
        if (!RewardLine.IsEmpty()) Lines.Add(RewardLine);
        if (!HatLine.IsEmpty()) Lines.Add(HatLine);

        if (Lines.Num() == 0) return FText::GetEmpty();
        if (Lines.Num() == 1) return Lines[0];

        FText Result = Lines[0];
        for (int32 i = 1; i < Lines.Num(); i++)
            Result = FText::Format(NSLOCTEXT("Quest", "SummaryJoin", "{0}\n{1}"), Result, Lines[i]);
        return Result;
    }

    FText GetNotReadyText() const
    {
        if (!RequiredItem)
            return NSLOCTEXT("Quest", "NotReady", "You haven't completed what I asked.");

        return FText::Format(
            NSLOCTEXT("Quest", "NotReadyItem", "You haven't brought me the {0} yet. I need {1}."),
            RequiredItem->DisplayName,
            FText::AsNumber(RequiredQty));
    }
};