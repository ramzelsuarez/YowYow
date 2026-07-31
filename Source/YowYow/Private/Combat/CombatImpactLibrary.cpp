#include "Combat/CombatImpactLibrary.h"

#include "Combat/CombatImpactSubsystem.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"

void UCombatImpactLibrary::ApplyKnockback(
	AActor* Target,
	FVector Direction,
	float Strength,
	bool bHorizontalOnly,
	bool bOverrideXY,
	bool bOverrideZ
)
{
	if (!IsValid(Target) || Strength <= 0.f)
	{
		return;
	}

	FVector LaunchDirection = Direction;
	if (bHorizontalOnly)
	{
		LaunchDirection.Z = 0.f;
	}

	if (LaunchDirection.IsNearlyZero())
	{
		return;
	}

	LaunchDirection.Normalize();
	const FVector LaunchVelocity = LaunchDirection * Strength;

	if (ACharacter* Character = Cast<ACharacter>(Target))
	{
		Character->LaunchCharacter(LaunchVelocity, bOverrideXY, bOverrideZ);
		return;
	}

	if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Target->GetRootComponent()))
	{
		if (RootPrimitive->IsSimulatingPhysics())
		{
			RootPrimitive->AddImpulse(LaunchVelocity, NAME_None, true);
		}
	}
}

void UCombatImpactLibrary::ApplyHitStop(AActor* Target, float Duration, float TimeDilation)
{
	if (!IsValid(Target) || Duration <= 0.f)
	{
		return;
	}

	UWorld* World = Target->GetWorld();
	if (!World)
	{
		return;
	}

	if (UCombatImpactSubsystem* Subsystem = World->GetSubsystem<UCombatImpactSubsystem>())
	{
		Subsystem->ApplyHitStop(Target, Duration, TimeDilation);
	}
}

void UCombatImpactLibrary::ApplyHitStopPair(
	AActor* Attacker,
	AActor* Victim,
	float Duration,
	float TimeDilation
)
{
	ApplyHitStop(Attacker, Duration, TimeDilation);
	ApplyHitStop(Victim, Duration, TimeDilation);
}
