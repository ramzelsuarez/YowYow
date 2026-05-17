#pragma once

#include "CoreMinimal.h"
#include "HomingStates.generated.h"

UENUM(BlueprintType)
enum class EHomingState : uint8 {
	Idle UMETA(DisplayName = "Idle"),

	// in air, searching targets
	Searching UMETA(DisplayName = "Searching"),

	// in air, target found
	TargetFound UMETA(DisplayName = "TargetFound"),

	// actually doing the homing motion
	Launching UMETA(DisplayName = "Launching"),

	// on hit
	Hit UMETA(DisplayName = "Hit"),

	// after hitting, on cooldown
	Recovery UMETA(DisplayName = "Recovery"),
};
