// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterStates/HomingStates.h"
#include "Components/ActorComponent.h"
#include "HomingAttackComponent.generated.h"

class UCharacterStateComponent;
class ACharacterBase;
class UPawnMovementComponent;

/** Consumed by AEriCharacter (camera unlock + action/locomotion after bounce or cancel). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnHomingAttackFinished,
	bool, bSuccess
);

/**
 * Homing attack: search while airborne, dash to target, bounce on hit.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class YOWYOW_API UHomingAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHomingAttackComponent();

	UFUNCTION(BlueprintCallable)
	bool CanSearchTargets();

	UFUNCTION(BlueprintCallable)
	EHomingState GetHomingState() const { return HomingState; }

	UFUNCTION(BlueprintCallable)
	void DoHomingAttack();

	/** Abort in-flight homing (grounded, lost target, etc.). Broadcasts finished(false). */
	UFUNCTION(BlueprintCallable)
	void CancelHomingAttack();

	UFUNCTION(BlueprintPure, Category = "Homing")
	AActor* GetCurrentHomingTarget() const { return CurrentTarget; }

	UFUNCTION(BlueprintPure, Category = "Homing")
	bool IsHomingInFlight() const;

	UPROPERTY(BlueprintAssignable)
	FOnHomingAttackFinished OnHomingAttackFinished;

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	ACharacterBase* OwnerCharacter;

	UPROPERTY()
	UCharacterStateComponent* OwnerStateComponent;

	UPROPERTY()
	UPawnMovementComponent* OwnerMovementComponent;

	void FindTargets(TArray<AActor*>& OutTargets);
	bool GetBestTarget();
	void UpdateHomingAttack();
	void SetCurrentTarget(AActor* NewTarget);
	void ClearTarget();
	void FinishHomingAttack();
	void ProcessRecoveryState();

	UPROPERTY()
	AActor* CurrentTarget = nullptr;

	UPROPERTY(EditAnywhere)
	float SearchRadius = 1200.f;

	UPROPERTY(EditAnywhere)
	float InitialHomingSpeed = 1000.f;

	UPROPERTY(EditAnywhere)
	float HitDistance = 20.f;

	UPROPERTY(EditAnywhere)
	float HitBounceSpeed = 2000.f;

	UPROPERTY(EditAnywhere)
	float HomingCooldown = 2.f;

	TArray<TEnumAsByte<EObjectTypeQuery>> TargetObjectType = {
		UEngineTypes::ConvertToObjectType(ECC_WorldDynamic),
		UEngineTypes::ConvertToObjectType(ECC_WorldStatic),
		UEngineTypes::ConvertToObjectType(ECC_Pawn)
	};

private:
	EHomingState HomingState = EHomingState::Idle;
	FTimerHandle HomingCooldownTimer;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
