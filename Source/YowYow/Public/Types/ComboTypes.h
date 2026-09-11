#pragma once

#include "CoreMinimal.h"
#include "ComboTypes.generated.h"

UENUM(BlueprintType)
enum class EComboTier : uint8
{
	None UMETA(DisplayName = "None"),
	D UMETA(DisplayName = "D"),
	C UMETA(DisplayName = "C"),
	B UMETA(DisplayName = "B"),
	A UMETA(DisplayName = "A"),
	S UMETA(DisplayName = "S"),
	SS UMETA(DisplayName = "SS"),
	SSS UMETA(DisplayName = "SSS")
};

USTRUCT(BlueprintType)
struct FComboTierDef
{
	GENERATED_BODY()

	FComboTierDef() = default;

	FComboTierDef(EComboTier InTier, int32 InMinPoints, FLinearColor InColor)
		: Tier(InTier)
		, MinPoints(InMinPoints)
		, Color(InColor)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EComboTier Tier = EComboTier::D;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
	int32 MinPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor Color = FLinearColor::White;
};
