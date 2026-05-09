// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterStates/CharacterStates.h"
#include "Components/ActorComponent.h"
#include "CharacterStateComponent.generated.h"

/**
 * this component should only handle logic of the states
 * if at any point the change to some other state depends on some other different state or character's variables
 * that should be evaluated in the moment and place of transition, not here
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class YOWYOW_API UCharacterStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCharacterStateComponent();

	UFUNCTION(BlueprintPure)
	ECharacterActionState GetActionState() const { return ActionState; };

	UFUNCTION(BlueprintCallable)
	void SetActionState(ECharacterActionState NewActionState);
	
	UFUNCTION(BlueprintPure)
	ECharacterLocomotionState GetLocomotionState() const { return LocomotionState; };

	UFUNCTION(BlueprintCallable)
	void SetLocomotionState(ECharacterLocomotionState NewLocomotionState);
	
	UFUNCTION(BlueprintPure)
	ECharacterLifeState GetLifeState() const { return LifeState; };

	UFUNCTION(BlueprintCallable)
	void SetLifeState(ECharacterLifeState NewLifeState);
	
	UFUNCTION(BlueprintPure)
	ECharacterAttackState GetAttackState() const { return AttackState; };

	UFUNCTION(BlueprintCallable)
	void SetAttackState(ECharacterAttackState NewAttackState);
	

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	ECharacterActionState ActionState = ECharacterActionState::Normal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	ECharacterLocomotionState LocomotionState = ECharacterLocomotionState::Grounded;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	ECharacterLifeState LifeState = ECharacterLifeState::Alive;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	ECharacterAttackState AttackState = ECharacterAttackState::None;
};
