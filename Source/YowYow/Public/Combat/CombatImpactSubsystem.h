#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CombatImpactSubsystem.generated.h"

/**
 * Owns per-actor hitstop timers so restores survive hitbox destruction.
 */
UCLASS()
class YOWYOW_API UCombatImpactSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Applies local time dilation and schedules restore. Replaces any pending hitstop on Target. */
	void ApplyHitStop(AActor* Target, float Duration, float TimeDilation);

	/** Cancels pending restore and restores dilation immediately. */
	void ClearHitStop(AActor* Target);

	virtual void Deinitialize() override;

private:
	struct FHitStopEntry
	{
		FTimerHandle TimerHandle;
		float PreviousDilation = 1.f;
	};

	TMap<TWeakObjectPtr<AActor>, FHitStopEntry> ActiveHitStops;

	void RestoreTimeDilation(TWeakObjectPtr<AActor> Target);
};
