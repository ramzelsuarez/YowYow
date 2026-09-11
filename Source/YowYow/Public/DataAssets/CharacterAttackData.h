// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/AttackTypes.h"
#include "CharacterAttackData.generated.h"

/**
 * 
 */
UCLASS()
class YOWYOW_API UCharacterAttackData : public UDataAsset
{
	GENERATED_BODY()

public:
	UCharacterAttackData()
	{
		Area.Motion = EAttackMotion::OrbitCircle;
		Area.YoYoHand = EYoYoHand::Both;
		DNA.Motion = EAttackMotion::OrbitOwner;
		DNA.YoYoHand = EYoYoHand::Both;
		DNA.Range = 150.f;
	}
	/*
	 * "Normal" is the only attack type that can be combo-ed, therefore it's an array of attacks
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FAttackData> Normal;
	
	/*
	 * Area attack is a single one and done attack, therefore no array
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAttackData Area;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRangedAttackData Ranged;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DNA")
	FAttackData DNA;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DNA", meta = (ClampMin = "0.1"))
	float DNAPhase1Duration = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DNA", meta = (ClampMin = "6", ClampMax = "6"))
	int32 DNABurstCount = 6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DNA", meta = (ClampMin = "1.0"))
	float DNABurstRange = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DNA", meta = (ClampMin = "1.0"))
	float DNABurstSpeed = 800.f;
};
