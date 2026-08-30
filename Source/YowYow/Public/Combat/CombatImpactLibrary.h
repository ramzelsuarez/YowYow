#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CombatImpactLibrary.generated.h"

/**
 * Shared combat feel helpers (knockback + hitstop). Hitstop state lives in UCombatImpactSubsystem.
 */
UCLASS()
class YOWYOW_API UCombatImpactLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Combat|Impact")
	static void ApplyKnockback(
		AActor* Target,
		FVector Direction,
		float Strength,
		bool bHorizontalOnly = true,
		bool bOverrideXY = true,
		bool bOverrideZ = false
	);

	UFUNCTION(BlueprintCallable, Category = "Combat|Impact")
	static void ApplyHitStop(
		AActor* Target,
		float Duration,
		float TimeDilation = 0.05f
	);

	UFUNCTION(BlueprintCallable, Category = "Combat|Impact")
	static void ApplyHitStopPair(
		AActor* Attacker,
		AActor* Victim,
		float Duration,
		float TimeDilation = 0.05f
	);
};
