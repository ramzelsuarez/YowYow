// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/HealthComponent.h"

UHealthComponent::UHealthComponent()
{
	// never tick on health. Manage with delegates
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

void UHealthComponent::Heal(int8 Amount)
{
	if (Amount <= 0 || bIsDead) return;

	const int8 PreviousHealth = CurrentHealth;

	CurrentHealth = CurrentHealth + Amount;

	const int8 DeltaHealth = CurrentHealth - PreviousHealth;

	if (DeltaHealth > 0)
	{
		OnHealthChanged.Broadcast(this, CurrentHealth, MaxHealth, DeltaHealth);
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
	if (!DamagedActor || bIsDead || Damage < 0) return;
	
	const float PreviousHealth = CurrentHealth;
	
	CurrentHealth = CurrentHealth - Damage;
	
	const float DeltaHealth = CurrentHealth - PreviousHealth;
	
	if (DeltaHealth < 0) return;
	
	OnHealthChanged.Broadcast(this, CurrentHealth, MaxHealth, -DeltaHealth);
	
	if (CurrentHealth < 0)
	{
		bIsDead = true;
		OnHealthDepleted.Broadcast(this, DamageCauser);
	}
}
