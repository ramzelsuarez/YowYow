#pragma once

#include "CoreMinimal.h"
#include "CharacterStates.generated.h"

/*
 * General action states
 */
UENUM(BlueprintType)
enum class ECharacterActionState : uint8
{
	Normal UMETA(DisplayName = "Normal"),

	// Trick and homing are only used by Eri
	Trick UMETA(DisplayName = "Trick"),
	Homing UMETA(DisplayName = "Homing"),
};

/*
 * Generic life state shared by player/enemies
 */
UENUM(BlueprintType)
enum class ECharacterLifeState : uint8
{
	Alive UMETA(DisplayName = "Alive"),
	Dead UMETA(DisplayName = "Dead"),
};

/*
 * Generic physical locomotion state
 * Idle/Run can usually be derived in the AnimBP from Grounded + Speed
 */
UENUM(BlueprintType)
enum class ECharacterLocomotionState : uint8
{
	Grounded UMETA(DisplayName = "Grounded"),
	Jumping UMETA(DisplayName = "Jumping"),
	Falling UMETA(DisplayName = "Falling"),
};

/*
 * Generic attack phase state
 */
UENUM(BlueprintType)
enum class ECharacterAttackState : uint8
{
	None UMETA(DisplayName = "None"),
	// while the attack is active
	Attacking UMETA(DisplayName = "Attacking"),
	// when the character no longer can combo into an attack but can't move yet
	Recovery UMETA(DisplayName = "Recovery"),
};