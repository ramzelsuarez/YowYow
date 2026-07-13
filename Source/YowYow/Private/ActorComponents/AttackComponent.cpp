// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/AttackComponent.h"

#include "ActorComponents/CharacterStateComponent.h"
#include "Attacks/AttackHitbox.h"
#include "Characters/CharacterBase.h"
#include "Characters/EriCharacter.h"
#include "DataAssets/CharacterAttackData.h"
#include "Engine/World.h"
#include "TimerManager.h"

UAttackComponent::UAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UAttackComponent::TryAttack(EAttackType AttackType)
{
	if (IsValid(ActiveHitbox) || !GetWorld())
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
	if (!SelectedAttack || !ExecuteMeleeAttack(*SelectedAttack, AttackType))
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
	}

	return true;
}

void UAttackComponent::SetAttachedHitboxSource(USceneComponent* HitboxSource)
{
	AttachedHitboxSource = HitboxSource;
}

void UAttackComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAttackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ResetNormalCombo();
	Super::EndPlay(EndPlayReason);
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

bool UAttackComponent::ExecuteMeleeAttack(const FAttackData& AttackData, EAttackType AttackType)
{
	USceneComponent* HitboxSource = AttachedHitboxSource;

	if (AttackType == EAttackType::Normal && Cast<AEriCharacter>(GetOwner()))
	{
		AEriCharacter* EriCharacter = Cast<AEriCharacter>(GetOwner());
		if (EriCharacter && EriCharacter->GetYoYoHitboxSource())
		{
			if (!EriCharacter->BeginYoYoAttack(AttackData))
			{
				return false;
			}

			HitboxSource = EriCharacter->GetYoYoHitboxSource();
		}
	}

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
	ActiveHitbox->Initialize(GetOwner(), AttackData, HitboxRadius, HitboxSource);
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

	// Projectile lifetime and hit behavior belong to the projectile class.
	ResetNormalCombo();
	FinishAttack();
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
	if (FinishedHitbox != ActiveHitbox)
	{
		return;
	}

	ActiveHitbox = nullptr;
	FinishAttack();
}

void UAttackComponent::FinishAttack()
{
	if (bActiveAttackIsNormal)
	{
		RestartNormalComboTimer();
		bActiveAttackIsNormal = false;
	}

	if (UCharacterStateComponent* StateComponent = GetOwner()->FindComponentByClass<UCharacterStateComponent>())
	{
		StateComponent->SetAttackState(ECharacterAttackState::None);
	}
}
