// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/CharacterStateComponent.h"

// Sets default values for this component's properties
UCharacterStateComponent::UCharacterStateComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UCharacterStateComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCharacterStateComponent::SetActionState(ECharacterActionState NewActionState)
{
	ActionState = NewActionState;
}

void UCharacterStateComponent::SetLocomotionState(ECharacterLocomotionState NewLocomotionState)
{
	LocomotionState = NewLocomotionState;
}

void UCharacterStateComponent::SetLifeState(ECharacterLifeState NewLifeState)
{
	LifeState = NewLifeState;
}

void UCharacterStateComponent::SetAttackState(ECharacterAttackState NewAttackState)
{
	AttackState = NewAttackState;
}

// Called every frame
void UCharacterStateComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
