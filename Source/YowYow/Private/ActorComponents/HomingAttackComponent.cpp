// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/HomingAttackComponent.h"

#include "ActorComponents/CharacterStateComponent.h"
#include "Characters/CharacterBase.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Interfaces/Homingable.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"
#include "Engine/World.h"

// Sets default values for this component's properties
UHomingAttackComponent::UHomingAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

bool UHomingAttackComponent::CanSearchTargets()
{
	if (!OwnerStateComponent)
	{
		return false;
	}

	return OwnerStateComponent->GetLocomotionState() == ECharacterLocomotionState::Airborne &&
		OwnerStateComponent->GetLifeState() != ECharacterLifeState::Dead &&
		(HomingState == EHomingState::Idle ||
			HomingState == EHomingState::Searching ||
			HomingState == EHomingState::TargetFound);
}

void UHomingAttackComponent::DoHomingAttack()
{
	if (IsValid(CurrentTarget))
	{
		HomingState = EHomingState::Launching;

		const FVector TargetLocation = IHomingable::Execute_GetTargetLocation(CurrentTarget);
		const FVector Direction = (TargetLocation - OwnerCharacter->GetActorLocation()).GetSafeNormal();

		OwnerCharacter->LaunchCharacter(Direction * InitialHomingSpeed, true, true);
	}
}

void UHomingAttackComponent::UpdateHomingAttack()
{
	if (IsValid(CurrentTarget))
	{
		const FVector TargetLocation = IHomingable::Execute_GetTargetLocation(CurrentTarget);
		const FVector Direction = (TargetLocation - OwnerCharacter->GetActorLocation()).GetSafeNormal();
		const FVector DistanceToTarget = TargetLocation - OwnerCharacter->GetActorLocation();

		if (DistanceToTarget.SizeSquared() < FMath::Square(HitDistance))
		{
			HomingState = EHomingState::Hit;
			return;
		}

		OwnerMovementComponent->Velocity = Direction * InitialHomingSpeed;
		return;
	}

	ClearTarget();
	HomingState = EHomingState::Idle;
}

void UHomingAttackComponent::SetCurrentTarget(AActor* NewTarget)
{
	if (CurrentTarget == NewTarget)
	{
		return;
	}

	if (IsValid(CurrentTarget) && CurrentTarget->Implements<UHomingable>())
	{
		IHomingable::Execute_SetHomingTargeted(CurrentTarget, false);
	}

	CurrentTarget = NewTarget;

	if (IsValid(CurrentTarget) && CurrentTarget->Implements<UHomingable>())
	{
		IHomingable::Execute_SetHomingTargeted(CurrentTarget, true);
	}
}

void UHomingAttackComponent::ClearTarget()
{
	SetCurrentTarget(nullptr);
}

void UHomingAttackComponent::FinishHomingAttack()
{
	FVector const Direction = FVector(0, 0, HitBounceSpeed);

	OwnerCharacter->LaunchCharacter(Direction, false, true);

	HomingState = EHomingState::Recovery;
	ClearTarget();
	OnHomingAttackFinished.Broadcast(true);

	GetWorld()->GetTimerManager().SetTimer(
		HomingCooldownTimer,
		this,
		&UHomingAttackComponent::ProcessRecoveryState,
		HomingCooldown,
		false
	);
}

void UHomingAttackComponent::ProcessRecoveryState()
{
	HomingState = EHomingState::Idle;
}

void UHomingAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacterBase>(GetOwner());

	if (!OwnerCharacter)
	{
		SetComponentTickEnabled(false);
		return;
	}

	OwnerStateComponent = OwnerCharacter->GetComponentByClass<UCharacterStateComponent>();

	OwnerMovementComponent = OwnerCharacter->GetMovementComponent();
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
		SetCurrentTarget(BestTarget);
		return true;
	}

	ClearTarget();
	return false;
}

void UHomingAttackComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerCharacter || !OwnerStateComponent || !OwnerMovementComponent)
	{
		return;
	}

	const bool bCanHomingExist =
		OwnerStateComponent->GetLocomotionState() == ECharacterLocomotionState::Airborne &&
		OwnerStateComponent->GetLifeState() != ECharacterLifeState::Dead;

	if (!bCanHomingExist)
	{
		HomingState = EHomingState::Idle;
		ClearTarget();
		return;
	}

	switch (HomingState)
	{
	case EHomingState::Idle:
	case EHomingState::Searching:
	case EHomingState::TargetFound:
		HomingState = EHomingState::Searching;
		HomingState = GetBestTarget() ? EHomingState::TargetFound : EHomingState::Searching;
		break;

	case EHomingState::Launching:
		UpdateHomingAttack();
		break;

	case EHomingState::Hit:
		FinishHomingAttack();
		break;

	case EHomingState::Recovery:
		break;

	default:
		break;
	}
}
