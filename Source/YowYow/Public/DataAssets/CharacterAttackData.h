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
};
