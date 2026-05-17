// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterStates/HomingStates.h"
#include "Components/ActorComponent.h"
#include "HomingAttackComponent.generated.h"

class UCharacterStateComponent;
class ACharacterBase;

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

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY()
	ACharacterBase* OwnerCharacter;

	UCharacterStateComponent* OwnerStateComponent;
	
	UPawnMovementComponent* OwnerMovementComponent;

	void FindTargets(TArray<AActor*>& OutTargets);
	
	bool GetBestTarget();

	AActor* CurrentTarget = nullptr;

	// un-uproperty this, only added for iteration
	UPROPERTY(EditAnywhere)
	float SearchRadius = 1200.f;

	// I don't think this needs to be editable anywhere
	TArray<TEnumAsByte<EObjectTypeQuery>> TargetObjectType = {
		UEngineTypes::ConvertToObjectType(ECC_WorldDynamic),
		UEngineTypes::ConvertToObjectType(ECC_WorldStatic),
		UEngineTypes::ConvertToObjectType(ECC_Pawn)
	};

private:
	EHomingState HomingState = EHomingState::Idle;

	TArray<AActor*> Candidates;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
