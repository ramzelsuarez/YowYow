// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/HealthComponent.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	bIsDead = CurrentHealth <= 0;

	if (AActor* Owner = GetOwner())
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleOwnerTakeAnyDamage);
	}
}

void UHealthComponent::Heal(int32 Amount)
{
	if (Amount <= 0 || bIsDead)
	{
		return;
	}

	const int32 PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Min(CurrentHealth + Amount, MaxHealth);

	const int32 DeltaHealth = CurrentHealth - PreviousHealth;
	if (DeltaHealth > 0)
	{
		OnHealthChanged.Broadcast(this, CurrentHealth, MaxHealth, static_cast<float>(DeltaHealth));
	}
}

void UHealthComponent::HandleOwnerTakeAnyDamage(
	AActor* DamagedActor,
	float Damage,
	const UDamageType* DamageType,
	AController* InstigatedBy,
	AActor* DamageCauser
)
{
	if (!DamagedActor || bIsDead || Damage <= 0.f)
	{
		return;
	}

	const int32 PreviousHealth = CurrentHealth;
	const int32 DamageInt = FMath::Max(1, FMath::RoundToInt(Damage));
	CurrentHealth = FMath::Max(0, PreviousHealth - DamageInt);

	const int32 AppliedDamage = PreviousHealth - CurrentHealth;
	if (AppliedDamage <= 0)
	{
		return;
	}

	OnHealthChanged.Broadcast(this, CurrentHealth, MaxHealth, static_cast<float>(-AppliedDamage));
	OnHealthDamageTaken.Broadcast(
		this,
		AppliedDamage,
		CurrentHealth,
		DamageCauser,
		InstigatedBy
	);

	if (CurrentHealth <= 0)
	{
		bIsDead = true;
		OnHealthDepleted.Broadcast(this, DamageCauser);
	}
}
