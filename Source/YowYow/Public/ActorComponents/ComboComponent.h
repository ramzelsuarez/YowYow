#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/ComboTypes.h"
#include "ComboComponent.generated.h"

class UHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnComboChanged,
	int32, Points,
	int32, HitCount,
	EComboTier, Tier
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnComboTierChanged,
	EComboTier, OldTier,
	EComboTier, NewTier
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComboBroken);

/**
 * Stylish-rank combo meter (DMC-style). Player-only.
 * Hits on IComboable actors add points; idle drain drops the tier; taking damage drops one rank.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class YOWYOW_API UComboComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UComboComponent();

	/**
	 * Add HitPoints on InstigatorPawn's meter if HitActor implements IComboable.
	 * Caller must snapshot CanGrantCombo() *before* ApplyDamage (killing blows still count).
	 */
	static void NotifyHit(AActor* InstigatorPawn, AActor* HitActor);

	UFUNCTION(BlueprintCallable, Category = "Combo")
	void RegisterHit(int32 Points);

	UFUNCTION(BlueprintCallable, Category = "Combo")
	void DropTier();

	UFUNCTION(BlueprintCallable, Category = "Combo")
	void BreakCombo();

	UFUNCTION(BlueprintPure, Category = "Combo")
	int32 GetCurrentPoints() const { return FMath::FloorToInt(CurrentPoints); }

	UFUNCTION(BlueprintPure, Category = "Combo")
	int32 GetHitCount() const { return HitCount; }

	UFUNCTION(BlueprintPure, Category = "Combo")
	EComboTier GetCurrentTier() const { return CurrentTier; }

	UFUNCTION(BlueprintPure, Category = "Combo")
	int32 GetHitPoints() const { return HitPoints; }

	UFUNCTION(BlueprintPure, Category = "Combo")
	FText GetTierDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "Combo")
	FLinearColor GetTierColor() const;

	UFUNCTION(BlueprintPure, Category = "Combo")
	float GetProgressToNextTier() const;

	UFUNCTION(BlueprintPure, Category = "Combo")
	bool IsComboActive() const { return CurrentTier != EComboTier::None; }

	UPROPERTY(BlueprintAssignable, Category = "Combo")
	FOnComboChanged OnComboChanged;

	UPROPERTY(BlueprintAssignable, Category = "Combo")
	FOnComboTierChanged OnComboTierChanged;

	UPROPERTY(BlueprintAssignable, Category = "Combo")
	FOnComboBroken OnComboBroken;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo", meta = (ClampMin = "1"))
	int32 HitPoints = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo", meta = (ClampMin = "0.0"))
	float DecayDelay = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo", meta = (ClampMin = "0.0"))
	float DecayPerSecond = 35.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	TArray<FComboTierDef> Tiers;

	UPROPERTY(EditAnywhere, Category = "Combo|Debug")
	bool bDebugDrawCombo = true;

private:
	void BuildDefaultTiers();
	void SortTiers();
	void RecalculateTier();
	void BroadcastChanged() const;
	void DrawDebugCombo() const;
	bool IsOwnerDead() const;
	const FComboTierDef* FindTierDef(EComboTier Tier) const;
	EComboTier ComputeTierForPoints(float Points) const;

	UFUNCTION()
	void HandleOwnerHealthChanged(
		UHealthComponent* HealthComponent,
		int32 InCurrentHealth,
		int32 MaxHealth,
		float DeltaHealth
	);

	float CurrentPoints = 0.f;
	int32 HitCount = 0;
	EComboTier CurrentTier = EComboTier::None;
	float TimeSinceLastHit = 0.f;
};
