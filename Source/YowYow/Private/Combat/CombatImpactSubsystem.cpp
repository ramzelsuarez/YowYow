#include "Combat/CombatImpactSubsystem.h"

#include "Engine/World.h"
#include "TimerManager.h"

void UCombatImpactSubsystem::ApplyHitStop(AActor* Target, float Duration, float TimeDilation)
{
	if (!IsValid(Target) || Duration <= 0.f)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const TWeakObjectPtr<AActor> WeakTarget(Target);
	FHitStopEntry* Existing = ActiveHitStops.Find(WeakTarget);

	if (Existing)
	{
		World->GetTimerManager().ClearTimer(Existing->TimerHandle);
	}
	else
	{
		FHitStopEntry& NewEntry = ActiveHitStops.Add(WeakTarget);
		NewEntry.PreviousDilation = Target->CustomTimeDilation;
		Existing = &NewEntry;
	}

	Target->CustomTimeDilation = FMath::Clamp(TimeDilation, 0.f, 1.f);

	FTimerDelegate RestoreDelegate;
	RestoreDelegate.BindUObject(this, &UCombatImpactSubsystem::RestoreTimeDilation, WeakTarget);
	World->GetTimerManager().SetTimer(Existing->TimerHandle, RestoreDelegate, Duration, false);
}

void UCombatImpactSubsystem::ClearHitStop(AActor* Target)
{
	if (!IsValid(Target))
	{
		return;
	}

	const TWeakObjectPtr<AActor> WeakTarget(Target);
	FHitStopEntry* Existing = ActiveHitStops.Find(WeakTarget);
	if (!Existing)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(Existing->TimerHandle);
	}

	Target->CustomTimeDilation = Existing->PreviousDilation;
	ActiveHitStops.Remove(WeakTarget);
}

void UCombatImpactSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		for (TPair<TWeakObjectPtr<AActor>, FHitStopEntry>& Pair : ActiveHitStops)
		{
			World->GetTimerManager().ClearTimer(Pair.Value.TimerHandle);
			if (AActor* Actor = Pair.Key.Get())
			{
				Actor->CustomTimeDilation = Pair.Value.PreviousDilation;
			}
		}
	}

	ActiveHitStops.Reset();
	Super::Deinitialize();
}

void UCombatImpactSubsystem::RestoreTimeDilation(TWeakObjectPtr<AActor> Target)
{
	FHitStopEntry Entry;
	if (!ActiveHitStops.RemoveAndCopyValue(Target, Entry))
	{
		return;
	}

	if (AActor* Actor = Target.Get())
	{
		Actor->CustomTimeDilation = Entry.PreviousDilation;
	}
}
