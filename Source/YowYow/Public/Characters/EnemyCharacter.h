// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase.h"
#include "Interfaces/Homingable.h"
#include "EnemyCharacter.generated.h"

class AWaveEnemyManager;
class UHealthComponent;

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

public:
	virtual bool GetIsHomingTargeted_Implementation() override;
	virtual void SetHomingTargeted_Implementation(bool bTargeted) override;
	virtual bool CanBeHomed_Implementation() const override;
	virtual FVector GetTargetLocation_Implementation() override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Encounter", meta = (ClampMin = "0.0"))
	float CorpseLifetime = 1.2f;

private:
	bool bIsHomingTargeted = false;

	UPROPERTY()
	TObjectPtr<AWaveEnemyManager> WaveManager;

	FTimerHandle CorpseDestroyTimerHandle;

	UFUNCTION()
	void HandleEnemyHealthDepleted(UHealthComponent* InHealthComponent, AActor* DamageCauser);

	void DestroyCorpse();
};
