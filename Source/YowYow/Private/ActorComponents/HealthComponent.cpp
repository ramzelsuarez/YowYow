// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/HealthComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "GameModes/SpinningRiot.h"

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
	const ASpinningRiot* Demo = GetWorld() ? GetWorld()->GetAuthGameMode<ASpinningRiot>() : nullptr;
	if (!DamagedActor || bIsDead || Damage <= 0.f || IsInvulnerable()
		|| (Demo && Demo->GetDemoPhase() != EDemoPhase::Combat))
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

	if (const APawn* HealthPawn = Cast<APawn>(GetOwner()))
	{
		if (HealthPawn->IsPlayerControlled() && GetWorld())
		{
			InvulnerableUntil = GetWorld()->GetTimeSeconds() + FMath::Max(InvulnDuration, 0.f);
		}
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

bool UHealthComponent::IsInvulnerable() const
{
	const APawn* HealthPawn = Cast<APawn>(GetOwner());
	return HealthPawn && HealthPawn->IsPlayerControlled() && GetWorld()
		&& GetWorld()->GetTimeSeconds() < InvulnerableUntil;
}
