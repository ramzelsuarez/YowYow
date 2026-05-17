// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/HomingAttackComponent.h"

#include "ActorComponents/CharacterStateComponent.h"
#include "Characters/CharacterBase.h"
#include "Interfaces/Homingable.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values for this component's properties
UHomingAttackComponent::UHomingAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

bool UHomingAttackComponent::CanSearchTargets()
{
	return OwnerStateComponent->GetLocomotionState() == ECharacterLocomotionState::Airborne &&
		OwnerStateComponent->GetLifeState() != ECharacterLifeState::Dead &&
		HomingState == EHomingState::Searching;
}

void UHomingAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacterBase>(GetOwner());

	OwnerStateComponent = OwnerCharacter->GetComponentByClass<UCharacterStateComponent>();
}

void UHomingAttackComponent::FindTargets(TArray<AActor*>& OutTargets)
{
	TArray<AActor*> Overlaps;

	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		OwnerCharacter->GetActorLocation(),
		SearchRadius,
		TargetObjectType,
		nullptr,
		{OwnerCharacter},
		Overlaps
	);

	for (AActor* Actor : Overlaps)
	{
		if (!Actor) continue;

		if (Actor->Implements<UHomingable>())
		{
			if (IHomingable::Execute_CanBeHomed(Actor))
			{
				OutTargets.Add(Actor);
			}
		}
	}
}

bool UHomingAttackComponent::GetBestTarget()
{
	if (!OwnerCharacter)
	{
		return false;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PC)
	{
		return false;
	}

	TArray<AActor*> Candidates;
	FindTargets(Candidates);

	if (Candidates.Num() == 0)
	{
		return false;
	}

	int32 ViewportX, ViewportY;
	PC->GetViewportSize(ViewportX, ViewportY);

	const FVector2D ScreenCenter(
		ViewportX * 0.5f,
		ViewportY * 0.5f
	);

	AActor* BestTarget = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();

	for (AActor* Target : Candidates)
	{
		FVector TargetLocation = Target->GetActorLocation();


		if (Target->Implements<UHomingable>())
		{
			TargetLocation = IHomingable::Execute_GetTargetLocation(Target);
		}

		FVector2D ScreenPos;
		const bool bOnScreen =
			PC->ProjectWorldLocationToScreen(TargetLocation, ScreenPos);

		if (!bOnScreen)
			continue;

		// avoid extreme borders (probably fine though)
		if (ScreenPos.X < 0.f || ScreenPos.Y < 0.f ||
			ScreenPos.X > ViewportX || ScreenPos.Y > ViewportY)
			continue;

		const float DistSq =
			FVector2D::DistSquared(ScreenPos, ScreenCenter);

		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestTarget = Target;
		}
	}

	if (BestTarget)
	{
		CurrentTarget = BestTarget;
		return true;
	}

	return false;
}

void UHomingAttackComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CanSearchTargets())
	{
		HomingState = EHomingState::Searching;
		if (GetBestTarget())
		{
			HomingState = EHomingState::TargetFound;
		}
	}
	else
	{
		HomingState = EHomingState::Idle;
		CurrentTarget = nullptr;
	}
}
