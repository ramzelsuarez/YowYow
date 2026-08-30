#pragma once

#include "CoreMinimal.h"
#include "AttackTypes.generated.h"

UENUM(BlueprintType)
enum class EAttackType : uint8
{
	Normal UMETA(DisplayName = "Normal"),
	Area UMETA(DisplayName = "Area"),
	Ranged UMETA(DisplayName = "Ranged"),
};

UENUM(BlueprintType)
enum class EAttackMotion : uint8
{
	/** Semicircle sweep in front of the attacker (enemies). */
	ArcSweep UMETA(DisplayName = "Arc Sweep"),
	/** Hit volume follows a scene component (yoyo). */
	FollowSource UMETA(DisplayName = "Follow Source"),
	/**
	 * Heavy / area: dual crescents (medialunas).
	 * Both start behind the attacker and sweep once to the front —
	 * one through the right side, one through the left. No full spin.
	 */
	OrbitCircle UMETA(DisplayName = "Orbit Circle"),
};

UENUM(BlueprintType)
enum class EFollowSourceDamageWindow : uint8
{
	/** Damage only while the source moves outbound. */
	OutboundOnly UMETA(DisplayName = "Outbound Only"),
	/** Damage for the full configured duration. */
	FullPath UMETA(DisplayName = "Full Path"),
};

UENUM(BlueprintType)
enum class EYoYoHand : uint8
{
	Right UMETA(DisplayName = "Right"),
	Left UMETA(DisplayName = "Left"),
	Both UMETA(DisplayName = "Both"),
};

/** ABP attack pose: Throw holds last frame in-state; Catch is reverse. */
UENUM(BlueprintType)
enum class EYoYoAttackAnimPhase : uint8
{
	None UMETA(DisplayName = "None"),
	Throw UMETA(DisplayName = "Throw"),
	Catch UMETA(DisplayName = "Catch"),
};

USTRUCT(BlueprintType)
struct FAttackDataBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float Range = 100.f;

	/** Horizontal launch strength on hit (cm/s). 0 = no knockback. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float Knockback = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float Damage = 1.f;
};

USTRUCT(BlueprintType)
struct FAttackData : public FAttackDataBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float Speed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "1.0"))
	float HitboxRadius = 32.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EAttackMotion Motion = EAttackMotion::ArcSweep;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arc")
	float ArcStartDegrees = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arc")
	float ArcEndDegrees = -90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Follow Source")
	EFollowSourceDamageWindow FollowDamageWindow = EFollowSourceDamageWindow::OutboundOnly;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impact", meta = (ClampMin = "0.0"))
	float HitStopDuration = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impact", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HitStopDilation = 0.05f;

	/** Extra lockout after the hitbox ends (area anti-spam). 0 = none. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0.0"))
	float RecoveryTime = 0.f;

	/** Which yoyo(s) Eri should animate for this step (ignored by enemies). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YoYo")
	EYoYoHand YoYoHand = EYoYoHand::Right;
};

USTRUCT(BlueprintType)
struct FAreaAttackData
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FRangedAttackData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> Projectile;
};
