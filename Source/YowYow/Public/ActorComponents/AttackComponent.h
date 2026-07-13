// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TimerHandle.h"
#include "Types/AttackTypes.h"
#include "AttackComponent.generated.h"

class AAttackHitbox;
class UCharacterAttackData;
class USceneComponent;

/**
 * This component is meant to be used by both Eri and enemies
 *
 * it should handle:
 * - Setting the character state to attacking
 * - Checking for combo chains (a Data Asset should provide info on whether there are combo attacks left or not etc)
 * - Spawning generic melee hitboxes and ranged projectiles
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
	void SetAttachedHitboxSource(USceneComponent* HitboxSource);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	const FAttackData* GetAttackData(EAttackType AttackType);
	bool ExecuteMeleeAttack(const FAttackData& AttackData, EAttackType AttackType);
	bool ExecuteRangedAttack(const FRangedAttackData& AttackData);
	void ResetNormalCombo();
	void RestartNormalComboTimer();
	void FinishAttack();
	void HandleHitboxFinished(AAttackHitbox* FinishedHitbox);

	UPROPERTY(EditAnywhere, Category="Attack", meta=(ClampMin="1.0"))
	float HitboxRadius = 32.f;

	UPROPERTY()
	AAttackHitbox* ActiveHitbox = nullptr;

	UPROPERTY()
	USceneComponent* AttachedHitboxSource = nullptr;

	UPROPERTY(EditAnywhere, Category="Attack|Combo", meta=(ClampMin="0.0"))
	float ComboResetTime = 0.75f;

	FTimerHandle ComboResetTimer;
	int32 NormalAttackIndex = 0;
	bool bActiveAttackIsNormal = false;
};
