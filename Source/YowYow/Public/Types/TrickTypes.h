#pragma once

#include "CoreMinimal.h"
#include "TrickTypes.generated.h"

UENUM(BlueprintType)
enum class ETrickDirection : uint8
{
	None,
	Up,
	Down,
	Left,
	Right,
};
