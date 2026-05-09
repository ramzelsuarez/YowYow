#pragma once

#include "CoreMinimal.h"
#include "AttackTypes.generated.h"

UENUM(BlueprintType)
enum class EAttackType : uint8 {
	Normal UMETA(DisplayName = "Normal"),
	Area UMETA(DisplayName = "Area"),
};
