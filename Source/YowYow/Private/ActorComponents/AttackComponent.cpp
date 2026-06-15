// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/AttackComponent.h"

#include "ActorComponents/CharacterStateComponent.h"
#include "Attacks/AttackHitbox.h"
#include "Characters/CharacterBase.h"
#include "DataAssets/CharacterAttackData.h"
#include "Engine/World.h"
#include "Interfaces/AttackExecutor.h"

UAttackComponent::UAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UAttackComponent::TryAttack(EAttackType AttackType)
{
	if (IsValid(ActiveHitbox) || IsValid(ActiveAttackExecutor) || !GetWorld())
	{
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
	if (!SelectedAttack || !ExecuteMeleeAttack(*SelectedAttack))
	{
		return false;
	}

	if (AttackType == EAttackType::Normal)
	{
		++NormalAttackIndex;
	}

	return true;
}

void UAttackComponent::RegisterAttackExecutor(UActorComponent* Executor)
{
	if (Executor && Executor->GetOwner() == GetOwner() && Executor->Implements<UAttackExecutor>())
	{
		RegisteredAttackExecutor = Executor;
	}
}

void UAttackComponent::FinishExternalAttack(UActorComponent* Executor)
{
	if (Executor == ActiveAttackExecutor)
	{
		ActiveAttackExecutor = nullptr;
		FinishAttack();
	}
}

void UAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	TArray<UActorComponent*> OwnerComponents;
	GetOwner()->GetComponents(OwnerComponents);

	for (UActorComponent* Component : OwnerComponents)
	{
		if (Component && Component != this && Component->Implements<UAttackExecutor>())
		{
			RegisteredAttackExecutor = Component;
			break;
		}
	}
}

const FAttackData* UAttackComponent::GetAttackData(EAttackType AttackType)
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

		NormalAttackIndex %= CharacterAttackData->Normal.Num();
		return &CharacterAttackData->Normal[NormalAttackIndex];
	case EAttackType::Area:
		return &CharacterAttackData->Area;
	case EAttackType::Ranged:
	default:
		return nullptr;
	}
}

bool UAttackComponent::ExecuteMeleeAttack(const FAttackData& AttackData)
{
	if (ExecuteExternalAttack(AttackData))
	{
		return true;
	}

	if (AttackData.Shape == EAttackShape::Round)
	{
		return ExecuteRoundAttack(AttackData);
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("%s requires an AttackExecutor component for Straight attacks"),
		*GetOwner()->GetName()
	);
	return false;
}

bool UAttackComponent::ExecuteExternalAttack(const FAttackData& AttackData)
{
	if (!RegisteredAttackExecutor
		|| !IAttackExecutor::Execute_CanExecuteAttack(RegisteredAttackExecutor, AttackData))
	{
		return false;
	}

	ActiveAttackExecutor = RegisteredAttackExecutor;
	IAttackExecutor::Execute_ExecuteAttack(RegisteredAttackExecutor, this, AttackData);
	return true;
}

bool UAttackComponent::ExecuteRoundAttack(const FAttackData& AttackData)
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.Instigator = Cast<APawn>(GetOwner());
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ActiveHitbox = GetWorld()->SpawnActor<AAttackHitbox>(
		AAttackHitbox::StaticClass(),
		GetOwner()->GetActorTransform(),
		SpawnParameters
	);

	if (!ActiveHitbox)
	{
		return false;
	}

	ActiveHitbox->OnFinished.AddUObject(this, &UAttackComponent::HandleHitboxFinished);
	ActiveHitbox->Initialize(GetOwner(), AttackData, HitboxRadius);
	return true;
}

bool UAttackComponent::ExecuteRangedAttack(const FRangedAttackData& AttackData)
{
	if (!AttackData.Projectile)
	{
		return false;
	}

	const FVector SpawnLocation = GetOwner()->GetActorLocation()
		+ GetOwner()->GetActorTransform().TransformVectorNoScale(AttackData.SpawnOffset);

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

	// Projectile lifetime and hit behavior belong to the projectile class.
	FinishAttack();
	return true;
}

void UAttackComponent::HandleHitboxFinished(AAttackHitbox* FinishedHitbox)
{
	if (FinishedHitbox != ActiveHitbox)
	{
		return;
	}

	ActiveHitbox = nullptr;
	FinishAttack();
}

void UAttackComponent::FinishAttack()
{
	if (UCharacterStateComponent* StateComponent = GetOwner()->FindComponentByClass<UCharacterStateComponent>())
	{
		StateComponent->SetAttackState(ECharacterAttackState::None);
	}
}
