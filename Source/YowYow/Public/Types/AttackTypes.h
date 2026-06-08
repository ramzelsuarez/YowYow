#pragma once

#include "CoreMinimal.h"
#include "AttackTypes.generated.h"

UENUM(BlueprintType)
enum class EAttackType : uint8 {
	Normal UMETA(DisplayName = "Normal"),
	Area UMETA(DisplayName = "Area"),
	Ranged UMETA(DisplayName = "Ranged"),
};

UENUM(BlueprintType)
enum class EAttackShape : uint8
{
	Straight UMETA(DisplayName = "Straight"),
	Round UMETA(DisplayName = "Round"),
};

USTRUCT(BlueprintType)
struct FAttackDataBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Range;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Knockback;
};

USTRUCT(BlueprintType)
struct FAttackData : public FAttackDataBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EAttackShape Shape;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Speed;
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