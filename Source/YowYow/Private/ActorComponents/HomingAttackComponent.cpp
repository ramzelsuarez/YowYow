// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/HomingAttackComponent.h"

#include "ActorComponents/CharacterStateComponent.h"
#include "ActorComponents/ComboComponent.h"
#include "Characters/CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Interfaces/Comboable.h"
#include "Interfaces/Homingable.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"
#include "Engine/World.h"

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

bool UHomingAttackComponent::IsHomingInFlight() const
{
	return HomingState == EHomingState::Charging
		|| HomingState == EHomingState::Launching
		|| HomingState == EHomingState::Hit;
}

void UHomingAttackComponent::DoHomingAttack()
{
	if (!IsValid(CurrentTarget) || !OwnerCharacter)
	{
		return;
	}

	HomingState = EHomingState::Charging;
	SetChargingSuspended(true);
}

void UHomingAttackComponent::BeginLaunch()
{
	if (HomingState != EHomingState::Charging || !IsValid(CurrentTarget) || !OwnerCharacter)
	{
		return;
	}

	SetChargingSuspended(false);
	HomingState = EHomingState::Launching;

	const FVector TargetLocation = IHomingable::Execute_GetTargetLocation(CurrentTarget);
	const FVector Direction = (TargetLocation - OwnerCharacter->GetActorLocation()).GetSafeNormal();
	OwnerCharacter->LaunchCharacter(Direction * InitialHomingSpeed, true, true);
}

void UHomingAttackComponent::SetChargingSuspended(bool bSuspend)
{
	UCharacterMovementComponent* Movement = Cast<UCharacterMovementComponent>(OwnerMovementComponent);
	if (!Movement)
	{
		return;
	}

	if (bSuspend)
	{
		if (!bChargingSuspended)
		{
			CachedGravityScale = Movement->GravityScale;
		}
		bChargingSuspended = true;
		Movement->GravityScale = 0.f;
		Movement->StopMovementImmediately();
		Movement->Velocity = FVector::ZeroVector;
		return;
	}

	if (!bChargingSuspended)
	{
		return;
	}

	Movement->GravityScale = CachedGravityScale;
	bChargingSuspended = false;
}

void UHomingAttackComponent::CancelHomingAttack()
{
	if (!IsHomingInFlight() && HomingState != EHomingState::Charging)
	{
		// Still clear soft lock state if needed
		if (HomingState != EHomingState::Recovery)
		{
			ClearTarget();
			HomingState = EHomingState::Idle;
		}
		return;
	}

	SetChargingSuspended(false);
	ClearTarget();
	HomingState = EHomingState::Idle;
	OnHomingAttackFinished.Broadcast(false);
}

void UHomingAttackComponent::UpdateHomingAttack()
{
	if (IsValid(CurrentTarget) && OwnerCharacter && OwnerMovementComponent)
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

	// Lost target mid-flight
	CancelHomingAttack();
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

void UHomingAttackComponent::ApplyHomingHitDamage()
{
	if (!IsValid(CurrentTarget) || !OwnerCharacter)
	{
		return;
	}

	FVector TargetLocation = CurrentTarget->GetActorLocation();
	if (CurrentTarget->Implements<UHomingable>())
	{
		TargetLocation = IHomingable::Execute_GetTargetLocation(CurrentTarget);
	}

	const float HitRadius = HitDistance + HitRadiusThreshold;
	if (FVector::DistSquared(OwnerCharacter->GetActorLocation(), TargetLocation) > FMath::Square(HitRadius))
	{
		return;
	}

	AController* InstigatorController = OwnerCharacter->GetController();

	const bool bGrantsCombo =
		CurrentTarget->Implements<UComboable>() && IComboable::Execute_CanGrantCombo(CurrentTarget);

	UGameplayStatics::ApplyDamage(
		CurrentTarget,
		HomingDamage,
		InstigatorController,
		OwnerCharacter,
		nullptr
	);

	if (bGrantsCombo)
	{
		UComboComponent::NotifyHit(OwnerCharacter, CurrentTarget);
	}
}

void UHomingAttackComponent::FinishHomingAttack()
{
	if (!OwnerCharacter)
	{
		return;
	}

	ApplyHomingHitDamage();

	const FVector Direction = FVector(0, 0, HitBounceSpeed);
	OwnerCharacter->LaunchCharacter(Direction, false, true);

	HomingState = EHomingState::Recovery;
	ClearTarget();
	// Bounce: unlock camera / free look (Eri listens).
	OnHomingAttackFinished.Broadcast(true);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			HomingCooldownTimer,
			this,
			&UHomingAttackComponent::ProcessRecoveryState,
			HomingCooldown,
			false
		);
	}
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
		if (!Actor)
		{
			continue;
		}

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
		const bool bOnScreen = PC->ProjectWorldLocationToScreen(TargetLocation, ScreenPos);
		if (!bOnScreen)
		{
			continue;
		}

		if (ScreenPos.X < 0.f || ScreenPos.Y < 0.f ||
			ScreenPos.X > ViewportX || ScreenPos.Y > ViewportY)
		{
			continue;
		}

		const float DistSq = FVector2D::DistSquared(ScreenPos, ScreenCenter);
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
		if (IsHomingInFlight() || HomingState == EHomingState::Charging)
		{
			CancelHomingAttack();
		}
		else if (HomingState != EHomingState::Recovery)
		{
			HomingState = EHomingState::Idle;
			ClearTarget();
		}
		return;
	}

	switch (HomingState)
	{
	case EHomingState::Idle:
	case EHomingState::Searching:
	case EHomingState::TargetFound:
		HomingState = GetBestTarget() ? EHomingState::TargetFound : EHomingState::Searching;
		break;

	case EHomingState::Charging:
		if (UCharacterMovementComponent* Movement = Cast<UCharacterMovementComponent>(OwnerMovementComponent))
		{
			Movement->Velocity = FVector::ZeroVector;
		}
		if (!IsValid(CurrentTarget))
		{
			CancelHomingAttack();
		}
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
