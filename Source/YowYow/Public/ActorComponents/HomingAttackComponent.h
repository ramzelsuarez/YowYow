// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterStates/HomingStates.h"
#include "Components/ActorComponent.h"
#include "HomingAttackComponent.generated.h"

class UCharacterStateComponent;
class ACharacterBase;
class UPawnMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnHomingAttackFinished,
	bool, bSuccess
);

/**
 * This component should handle the homing attack functionality, state, etc
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class YOWYOW_API UHomingAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UHomingAttackComponent();

	UFUNCTION(BlueprintCallable)
	bool CanSearchTargets();

	UFUNCTION(BlueprintCallable)
	EHomingState GetHomingState() const { return HomingState; }

	UFUNCTION(BlueprintCallable)
	void DoHomingAttack();

	UPROPERTY(BlueprintAssignable)
	FOnHomingAttackFinished OnHomingAttackFinished;

protected:
	// Called when the game starts
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

	// un-uproperty following four vars, only added for iteration
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

	// I don't think this needs to be editable anywhere
	TArray<TEnumAsByte<EObjectTypeQuery>> TargetObjectType = {
		UEngineTypes::ConvertToObjectType(ECC_WorldDynamic),
		UEngineTypes::ConvertToObjectType(ECC_WorldStatic),
		UEngineTypes::ConvertToObjectType(ECC_Pawn)
	};

private:
	EHomingState HomingState = EHomingState::Idle;

	FTimerHandle HomingCooldownTimer;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
