#pragma once

#include "CoreMinimal.h"
#include "AttackTypes.generated.h"

UENUM(BlueprintType)
enum class EAttackType : uint8 {
	Normal UMETA(DisplayName = "Normal"),
	Area UMETA(DisplayName = "Area"),
	Ranged UMETA(DisplayName = "Ranged"),
};

USTRUCT(BlueprintType)
struct FAttackDataBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Range = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Knockback = 0.f;
};

USTRUCT(BlueprintType)
struct FAttackData : public FAttackDataBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Speed = 600.f;
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
