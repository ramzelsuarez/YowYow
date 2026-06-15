// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleSystem/WaveEnemyManager.h"

// Sets default values
AWaveEnemyManager::AWaveEnemyManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

bool AWaveEnemyManager::RequestAttackToken(AActor* RequestingEnemy)
{
	if (!RequestingEnemy)
	{
		return false;
	}

	if (EnemiesWithAttackTokens.Contains(RequestingEnemy))
	{
		return true;
	}

	if (EnemiesWithAttackTokens.Num() >= MaxAttackTokens)
	{
		return false;
	}

	EnemiesWithAttackTokens.Add(RequestingEnemy);
	return true;
}

void AWaveEnemyManager::ReleaseAttackToken(AActor* ReleasingEnemy)
{
	if (!ReleasingEnemy)
	{
		return;
	}

	EnemiesWithAttackTokens.Remove(ReleasingEnemy);
}

void AWaveEnemyManager::RegisterEnemyDefeated(AActor* DefeatedEnemy)
{
	ReleaseAttackToken(DefeatedEnemy);

	DefeatedEnemies++;

	UE_LOG(LogTemp, Warning, TEXT("Enemy defeated: %d / %d"), DefeatedEnemies, TotalEnemiesToDefeat);

	CheckWaveComplete();
}

void AWaveEnemyManager::CheckWaveComplete()
{
	if (DefeatedEnemies >= TotalEnemiesToDefeat)
	{
		UE_LOG(LogTemp, Warning, TEXT("Wave complete! Proceed to next area."));

		// Later:
		// Open gate
		// Trigger cutscene
		// Enable next area
	}
};

