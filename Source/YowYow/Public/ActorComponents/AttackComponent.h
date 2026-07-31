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

/** Consumed by AEriCharacter (yoyo presentation). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnAttackStarted,
	EAttackType, AttackType,
	FAttackData, StartedAttackData
);

/** Consumed by AEriCharacter (hit window ended — start yoyo return, do not snap home). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnAttackFinished,
	EAttackType, AttackType,
	bool, bCompleted
);

/**
 * Shared attack orchestrator for Eri and enemies.
 * While the yoyo is out / returning (presentation blocking), new inputs are buffered
 * and fire when the yoyo is fully home.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class YOWYOW_API UAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttackComponent();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool TryAttack(EAttackType AttackType = EAttackType::Normal);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetAttachedHitboxSource(USceneComponent* HitboxSource);

	UFUNCTION(BlueprintCallable, Category = "Combat|YoYo")
	void SetHandSources(USceneComponent* RightSource, USceneComponent* LeftSource);

	/**
	 * Eri: wait for full yoyo go+return before accepting the next free attack.
	 * Enemies leave this false (default).
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat|YoYo")
	void SetRequiresPresentationComplete(bool bRequires);

	/** Called by AEriCharacter when yoyos are fully home after a presentation. */
	UFUNCTION(BlueprintCallable, Category = "Combat|YoYo")
	void NotifyPresentationComplete();

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAttackActive() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsPresentationBlocking() const { return bPresentationBlocking; }

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnAttackStarted OnAttackStarted;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnAttackFinished OnAttackFinished;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	const FAttackData* GetAttackData(EAttackType AttackType) const;
	bool CanStartAttack() const;
	bool ExecuteMeleeAttack(const FAttackData& AttackData, EAttackType AttackType);
	bool ExecuteRangedAttack(const FRangedAttackData& AttackData);
	void CollectHitboxSources(const FAttackData& AttackData, TArray<USceneComponent*>& OutSources) const;
	bool SpawnHitbox(const FAttackData& AttackData, USceneComponent* SourceOrNull, float OrbitSideSign = -1.f);
	void ResetNormalCombo();
	void RestartNormalComboTimer();
	void BeginRecovery(float RecoverySeconds);
	void FinishRecovery();
	void FinishHitWindow(bool bCompleted);
	void CompleteAttackCycle(bool bCompleted);
	void HandleHitboxFinished(AAttackHitbox* FinishedHitbox);
	void ApplyAttackFacingLock(bool bLock);
	void SetAttackingStates(bool bAttacking);
	void TryConsumeBufferedAttack();

	UPROPERTY(EditAnywhere, Category = "Attack", meta = (ClampMin = "1.0"))
	float HitboxRadius = 32.f;

	UPROPERTY(EditAnywhere, Category = "Attack|Combo", meta = (ClampMin = "0.0"))
	float ComboResetTime = 0.75f;

	UPROPERTY()
	TArray<TObjectPtr<AAttackHitbox>> ActiveHitboxes;

	UPROPERTY()
	TObjectPtr<USceneComponent> AttachedHitboxSource = nullptr;

	UPROPERTY()
	TObjectPtr<USceneComponent> YoYoRightSource = nullptr;

	UPROPERTY()
	TObjectPtr<USceneComponent> YoYoLeftSource = nullptr;

	FTimerHandle ComboResetTimer;
	FTimerHandle RecoveryTimer;

	int32 NormalAttackIndex = 0;
	int32 PendingHitboxes = 0;
	bool bActiveAttackIsNormal = false;
	bool bInRecovery = false;
	bool bFacingLockedByAttack = false;
	bool bCachedOrientRotationToMovement = true;
	bool bRequiresPresentationComplete = false;
	bool bPresentationBlocking = false;
	bool bHasBufferedAttack = false;
	EAttackType BufferedAttackType = EAttackType::Normal;

	EAttackType ActiveAttackType = EAttackType::Normal;
	float ActiveRecoveryTime = 0.f;
};
