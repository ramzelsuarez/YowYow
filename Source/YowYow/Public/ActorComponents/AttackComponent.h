// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/AttackTypes.h"
#include "AttackComponent.generated.h"

class AAttackHitbox;
class UCharacterAttackData;

/**
 * This component is meant to be used by both Eri and enemies
 *
 * it should handle:
 * - Setting the character state to attacking
 * - Checking for combo chains (a Data Asset should provide info on whether there are combo attacks left or not etc)
 * - Delegating character-specific attacks to an IAttackExecutor component
 * - Spawning generic round hitboxes and ranged projectiles
 * - Optional: Tell the Character class that it should play an animation.
 *			   It's optional because maybe the character will handle this in ABP (as opposed to playing an AnimMontage).
 *			   Not sure how it's done with PaperZD 
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YOWYOW_API UAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttackComponent();

	UFUNCTION(BlueprintCallable)
	bool TryAttack(EAttackType AttackType = EAttackType::Normal);

	UFUNCTION(BlueprintCallable, Category="Combat")
	void RegisterAttackExecutor(UActorComponent* Executor);

	UFUNCTION(BlueprintCallable, Category="Combat")
	void FinishExternalAttack(UActorComponent* Executor);

protected:
	virtual void BeginPlay() override;

private:
	const FAttackData* GetAttackData(EAttackType AttackType);
	bool ExecuteMeleeAttack(const FAttackData& AttackData);
	bool ExecuteExternalAttack(const FAttackData& AttackData);
	bool ExecuteRoundAttack(const FAttackData& AttackData);
	bool ExecuteRangedAttack(const FRangedAttackData& AttackData);
	void FinishAttack();
	void HandleHitboxFinished(AAttackHitbox* FinishedHitbox);

	UPROPERTY(EditAnywhere, Category="Attack", meta=(ClampMin="1.0"))
	float HitboxRadius = 32.f;

	UPROPERTY()
	AAttackHitbox* ActiveHitbox = nullptr;

	UPROPERTY()
	UActorComponent* RegisteredAttackExecutor = nullptr;

	UPROPERTY()
	UActorComponent* ActiveAttackExecutor = nullptr;

	int32 NormalAttackIndex = 0;
};
