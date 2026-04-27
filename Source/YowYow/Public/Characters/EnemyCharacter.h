// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase.h"
#include "Interfaces/Homingable.h"
#include "EnemyCharacter.generated.h"

/**
 * Self explanatory class for all enemies (we will figure out later if a ABossCharacter inheriting from this one is really necessary)
 * Components this class should implement:
 * - AI Controller: self explanatory, altho the logic would live here
 * - Attacks/drops/etc should live within this class and consume a DataAsset to handle enemy-specific behavior
 */
UCLASS()
class YOWYOW_API AEnemyCharacter : public ACharacterBase, public IHomingable
{
	GENERATED_BODY()
	
};
