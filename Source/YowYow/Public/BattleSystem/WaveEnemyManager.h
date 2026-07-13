// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaveEnemyManager.generated.h"

UCLASS()
class YOWYOW_API AWaveEnemyManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AWaveEnemyManager();

	UFUNCTION(BlueprintCallable, Category = "Enemy Wave")
	bool RequestAttackToken(AActor* RequestingEnemy);

	UFUNCTION(BlueprintCallable, Category = "Enemy Wave")
	void ReleaseAttackToken(AActor* ReleasingEnemy);

	UFUNCTION(BlueprintCallable, Category = "Enemy Wave")
	void RegisterEnemyDefeated(AActor* DefeatedEnemy);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Wave")
	int32 TotalEnemiesToDefeat = 30;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Wave")
	int32 MaxAttackTokens = 3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy Wave")
	int32 DefeatedEnemies = 0;

	UPROPERTY()
	TArray<AActor*> EnemiesWithAttackTokens;
private:	
	// Called every frame
	void CheckWaveComplete();
};
