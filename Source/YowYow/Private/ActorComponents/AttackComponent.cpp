// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/AttackComponent.h"

#include "ActorComponents/CharacterStateComponent.h"
#include "ActorComponents/HealthComponent.h"
#include "Attacks/AttackHitbox.h"
#include "Characters/CharacterBase.h"
#include "DataAssets/CharacterAttackData.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

UAttackComponent::UAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UAttackComponent::IsAttackActive() const
{
	return PendingHitboxes > 0 || bInRecovery || bPresentationBlocking;
}

bool UAttackComponent::TryAttack(EAttackType AttackType)
{
	if (!GetWorld())
	{
		return false;
	}

	// Cache input while yoyo is still out / returning (or mid hit window).
	if (!CanStartAttack())
	{
		if (IsAttackActive() && (AttackType == EAttackType::Normal || AttackType == EAttackType::Area))
		{
			bHasBufferedAttack = true;
			BufferedAttackType = AttackType;
			return true;
		}
		return false;
	}

	const ACharacterBase* Character = Cast<ACharacterBase>(GetOwner());
	const UCharacterAttackData* CharacterAttackData = Character ? Character->AttackData : nullptr;
	if (!CharacterAttackData)
	{
		return false;
	}

	if (AttackType == EAttackType::Ranged)
	{
		return ExecuteRangedAttack(CharacterAttackData->Ranged);
	}

	const FAttackData* SelectedAttack = GetAttackData(AttackType);
	if (!SelectedAttack)
	{
		return false;
	}

	if (!ExecuteMeleeAttack(*SelectedAttack, AttackType))
	{
		return false;
	}

	if (AttackType == EAttackType::Normal)
	{
		++NormalAttackIndex;
		bActiveAttackIsNormal = true;
	}
	else
	{
		ResetNormalCombo();
		bActiveAttackIsNormal = false;
	}

	return true;
}

void UAttackComponent::SetAttachedHitboxSource(USceneComponent* HitboxSource)
{
	AttachedHitboxSource = HitboxSource;
}

void UAttackComponent::SetHandSources(USceneComponent* RightSource, USceneComponent* LeftSource)
{
	YoYoRightSource = RightSource;
	YoYoLeftSource = LeftSource;
}

void UAttackComponent::SetRequiresPresentationComplete(bool bRequires)
{
	bRequiresPresentationComplete = bRequires;
}

void UAttackComponent::NotifyPresentationComplete()
{
	bPresentationBlocking = false;

	// Yoyo may finish return before the hitbox window closes — wait for both.
	if (PendingHitboxes > 0 || bInRecovery)
	{
		return;
	}

	ApplyAttackFacingLock(false);
	SetAttackingStates(false);

	if (bActiveAttackIsNormal)
	{
		RestartNormalComboTimer();
		bActiveAttackIsNormal = false;
	}

	// Only now: start a buffered attack (combo input pressed during go/return).
	TryConsumeBufferedAttack();
}

void UAttackComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAttackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ComboResetTimer);
		World->GetTimerManager().ClearTimer(RecoveryTimer);
	}

	ResetNormalCombo();
	bPresentationBlocking = false;
	bHasBufferedAttack = false;
	ApplyAttackFacingLock(false);
	Super::EndPlay(EndPlayReason);
}

bool UAttackComponent::CanStartAttack() const
{
	if (!GetWorld() || PendingHitboxes > 0 || bInRecovery || bPresentationBlocking)
	{
		return false;
	}

	if (const UHealthComponent* Health = GetOwner()->FindComponentByClass<UHealthComponent>())
	{
		if (Health->IsDead())
		{
			return false;
		}
	}

	if (const UCharacterStateComponent* State = GetOwner()->FindComponentByClass<UCharacterStateComponent>())
	{
		if (State->GetLifeState() == ECharacterLifeState::Dead)
		{
			return false;
		}

		if (State->GetAttackState() != ECharacterAttackState::None)
		{
			return false;
		}
	}

	return true;
}

const FAttackData* UAttackComponent::GetAttackData(EAttackType AttackType) const
{
	const ACharacterBase* Character = Cast<ACharacterBase>(GetOwner());
	const UCharacterAttackData* CharacterAttackData = Character ? Character->AttackData : nullptr;
	if (!CharacterAttackData)
	{
		return nullptr;
	}

	switch (AttackType)
	{
	case EAttackType::Normal:
		if (CharacterAttackData->Normal.IsEmpty())
		{
			return nullptr;
		}
		return &CharacterAttackData->Normal[NormalAttackIndex % CharacterAttackData->Normal.Num()];
	case EAttackType::Area:
		return &CharacterAttackData->Area;
	case EAttackType::Ranged:
	default:
		return nullptr;
	}
}

void UAttackComponent::CollectHitboxSources(const FAttackData& AttackData, TArray<USceneComponent*>& OutSources) const
{
	OutSources.Reset();

	if (AttackData.Motion != EAttackMotion::FollowSource)
	{
		return;
	}

	auto AddUnique = [&OutSources](USceneComponent* Source)
	{
		if (IsValid(Source))
		{
			OutSources.AddUnique(Source);
		}
	};

	switch (AttackData.YoYoHand)
	{
	case EYoYoHand::Right:
		AddUnique(YoYoRightSource.Get());
		break;
	case EYoYoHand::Left:
		AddUnique(YoYoLeftSource.Get());
		break;
	case EYoYoHand::Both:
		AddUnique(YoYoRightSource.Get());
		AddUnique(YoYoLeftSource.Get());
		break;
	}

	if (OutSources.IsEmpty())
	{
		AddUnique(AttachedHitboxSource.Get());
		AddUnique(YoYoRightSource.Get());
		AddUnique(YoYoLeftSource.Get());
		if (OutSources.Num() > 1 && AttackData.YoYoHand != EYoYoHand::Both)
		{
			OutSources.SetNum(1);
		}
	}
}

bool UAttackComponent::SpawnHitbox(const FAttackData& AttackData, USceneComponent* SourceOrNull)
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.Instigator = Cast<APawn>(GetOwner());
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AAttackHitbox* Hitbox = GetWorld()->SpawnActor<AAttackHitbox>(
		AAttackHitbox::StaticClass(),
		GetOwner()->GetActorTransform(),
		SpawnParameters
	);

	if (!Hitbox)
	{
		return false;
	}

	Hitbox->OnFinished.AddUObject(this, &UAttackComponent::HandleHitboxFinished);
	Hitbox->Initialize(GetOwner(), AttackData, SourceOrNull);
	ActiveHitboxes.Add(Hitbox);
	++PendingHitboxes;
	return true;
}

bool UAttackComponent::ExecuteMeleeAttack(const FAttackData& AttackData, EAttackType AttackType)
{
	FAttackData ResolvedAttack = AttackData;
	if (ResolvedAttack.HitboxRadius <= 0.f)
	{
		ResolvedAttack.HitboxRadius = HitboxRadius;
	}

	if (AttackType == EAttackType::Area && ResolvedAttack.Motion == EAttackMotion::ArcSweep)
	{
		ResolvedAttack.Motion = EAttackMotion::OrbitCircle;
	}

	TArray<USceneComponent*> Sources;
	CollectHitboxSources(ResolvedAttack, Sources);

	if (ResolvedAttack.Motion == EAttackMotion::FollowSource)
	{
		if (Sources.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s FollowSource attack has no hitbox sources."),
				*GetNameSafe(GetOwner()));
			return false;
		}
	}

	ActiveAttackType = AttackType;
	ActiveRecoveryTime = ResolvedAttack.RecoveryTime;
	PendingHitboxes = 0;
	ActiveHitboxes.Reset();

	// Block next attack until yoyo fully returns (Eri sets RequiresPresentationComplete).
	const bool bUsesPresentation =
		bRequiresPresentationComplete
		&& (ResolvedAttack.Motion == EAttackMotion::FollowSource
			|| ResolvedAttack.Motion == EAttackMotion::OrbitCircle);
	bPresentationBlocking = bUsesPresentation;

	SetAttackingStates(true);
	ApplyAttackFacingLock(true);

	OnAttackStarted.Broadcast(AttackType, ResolvedAttack);

	bool bSpawnedAny = false;

	if (ResolvedAttack.Motion == EAttackMotion::FollowSource)
	{
		for (USceneComponent* Source : Sources)
		{
			if (SpawnHitbox(ResolvedAttack, Source))
			{
				bSpawnedAny = true;
			}
		}
	}
	else
	{
		bSpawnedAny = SpawnHitbox(ResolvedAttack, nullptr);
	}

	if (!bSpawnedAny)
	{
		PendingHitboxes = 0;
		ActiveHitboxes.Reset();
		bPresentationBlocking = false;
		ApplyAttackFacingLock(false);
		SetAttackingStates(false);
		OnAttackFinished.Broadcast(AttackType, false);
		return false;
	}

	return true;
}

bool UAttackComponent::ExecuteRangedAttack(const FRangedAttackData& AttackData)
{
	if (!AttackData.Projectile)
	{
		return false;
	}

	const FVector SpawnLocation = GetOwner()->GetActorLocation()
		+ GetOwner()->GetActorForwardVector() * 50.f
		+ FVector::UpVector * 50.f;

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.Instigator = Cast<APawn>(GetOwner());
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SpawnedProjectile = GetWorld()->SpawnActor<AActor>(
		AttackData.Projectile,
		SpawnLocation,
		GetOwner()->GetActorRotation(),
		SpawnParameters
	);

	if (!SpawnedProjectile)
	{
		return false;
	}

	ResetNormalCombo();
	ActiveAttackType = EAttackType::Ranged;
	ActiveRecoveryTime = 0.f;
	bPresentationBlocking = false;
	SetAttackingStates(true);
	OnAttackStarted.Broadcast(EAttackType::Ranged, FAttackData{});
	CompleteAttackCycle(true);
	return true;
}

void UAttackComponent::ResetNormalCombo()
{
	NormalAttackIndex = 0;

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ComboResetTimer);
	}

	bActiveAttackIsNormal = false;
}

void UAttackComponent::RestartNormalComboTimer()
{
	if (!GetWorld())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(ComboResetTimer);

	if (ComboResetTime <= 0.f)
	{
		ResetNormalCombo();
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(
		ComboResetTimer,
		this,
		&UAttackComponent::ResetNormalCombo,
		ComboResetTime,
		false
	);
}

void UAttackComponent::HandleHitboxFinished(AAttackHitbox* FinishedHitbox)
{
	ActiveHitboxes.Remove(FinishedHitbox);
	PendingHitboxes = FMath::Max(0, PendingHitboxes - 1);

	if (PendingHitboxes > 0)
	{
		return;
	}

	ActiveHitboxes.Reset();
	FinishHitWindow(true);
}

void UAttackComponent::FinishHitWindow(bool bCompleted)
{
	const EAttackType FinishedType = ActiveAttackType;

	// Tell presentation: hit window closed → start return (do not snap).
	OnAttackFinished.Broadcast(FinishedType, bCompleted);

	if (ActiveRecoveryTime > 0.f)
	{
		BeginRecovery(ActiveRecoveryTime);
		return;
	}

	if (bPresentationBlocking)
	{
		// Keep facing lock + attack state until NotifyPresentationComplete (yoyo home).
		if (UCharacterStateComponent* State = GetOwner()->FindComponentByClass<UCharacterStateComponent>())
		{
			State->SetAttackState(ECharacterAttackState::Recovery);
		}
		return;
	}

	CompleteAttackCycle(bCompleted);
}

void UAttackComponent::BeginRecovery(float RecoverySeconds)
{
	bInRecovery = true;

	if (UCharacterStateComponent* State = GetOwner()->FindComponentByClass<UCharacterStateComponent>())
	{
		State->SetAttackState(ECharacterAttackState::Recovery);
	}

	if (!GetWorld())
	{
		FinishRecovery();
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(RecoveryTimer);
	GetWorld()->GetTimerManager().SetTimer(
		RecoveryTimer,
		this,
		&UAttackComponent::FinishRecovery,
		RecoverySeconds,
		false
	);
}

void UAttackComponent::FinishRecovery()
{
	bInRecovery = false;
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(RecoveryTimer);
	}

	if (bPresentationBlocking)
	{
		// Still waiting for yoyo home.
		return;
	}

	CompleteAttackCycle(true);
}

void UAttackComponent::CompleteAttackCycle(bool bCompleted)
{
	if (bActiveAttackIsNormal && bCompleted && !bPresentationBlocking)
	{
		RestartNormalComboTimer();
		bActiveAttackIsNormal = false;
	}

	ActiveRecoveryTime = 0.f;
	bInRecovery = false;
	ApplyAttackFacingLock(false);
	SetAttackingStates(false);

	if (!bPresentationBlocking)
	{
		TryConsumeBufferedAttack();
	}
}

void UAttackComponent::TryConsumeBufferedAttack()
{
	if (!bHasBufferedAttack || !CanStartAttack())
	{
		return;
	}

	const EAttackType Type = BufferedAttackType;
	bHasBufferedAttack = false;
	TryAttack(Type);
}

void UAttackComponent::SetAttackingStates(bool bAttacking)
{
	UCharacterStateComponent* State = GetOwner()->FindComponentByClass<UCharacterStateComponent>();
	if (!State)
	{
		return;
	}

	if (bAttacking)
	{
		State->SetAttackState(ECharacterAttackState::Attacking);
		if (State->GetActionState() != ECharacterActionState::Homing
			&& State->GetActionState() != ECharacterActionState::Trick)
		{
			State->SetActionState(ECharacterActionState::Attacking);
		}
	}
	else
	{
		State->SetAttackState(ECharacterAttackState::None);
		if (State->GetActionState() == ECharacterActionState::Attacking)
		{
			State->SetActionState(ECharacterActionState::Default);
		}
	}
}

void UAttackComponent::ApplyAttackFacingLock(bool bLock)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Movement)
	{
		return;
	}

	if (bLock)
	{
		if (!bFacingLockedByAttack)
		{
			bCachedOrientRotationToMovement = Movement->bOrientRotationToMovement;
			bFacingLockedByAttack = true;
		}
		Movement->bOrientRotationToMovement = false;
	}
	else if (bFacingLockedByAttack)
	{
		Movement->bOrientRotationToMovement = bCachedOrientRotationToMovement;
		bFacingLockedByAttack = false;
	}
}
