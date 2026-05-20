// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/EnemyCharacter.h"

bool AEnemyCharacter::GetIsHomingTargeted_Implementation()
{
	return bIsHomingTargeted;
}

void AEnemyCharacter::SetHomingTargeted_Implementation(bool bTargeted)
{
	bIsHomingTargeted = bTargeted;
}

bool AEnemyCharacter::CanBeHomed_Implementation() const
{
	return true;
}

FVector AEnemyCharacter::GetTargetLocation_Implementation()
{
	return GetActorLocation();
}
