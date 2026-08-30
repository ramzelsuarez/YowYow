// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class AController;
class AActor;
class UHealthComponent;

/** Consumed by HUD (future) and debug; fires on heal and damage. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FOnHealthChanged,
	UHealthComponent*, HealthComponent,
	int32, CurrentHealth,
	int32, MaxHealth,
	float, DeltaHealth
);

/** Consumed by ACharacterBase / AEriCharacter (camera shake on player hurt). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(
	FOnHealthDamageTaken,
	UHealthComponent*, HealthComponent,
	int32, Damage,
	int32, CurrentHealth,
	AActor*, DamageCauser,
	AController*, DamageInstigator
);

/** Consumed by ACharacterBase (LifeState → Dead). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnHealthDepleted,
	UHealthComponent*, HealthComponent,
	AActor*, DamageCauser
);

/**
 * This component should provide bindings for health (value) and for its depletion and replenishment
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class YOWYOW_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	UPROPERTY(BlueprintAssignable)
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FOnHealthDamageTaken OnHealthDamageTaken;

	UPROPERTY(BlueprintAssignable)
	FOnHealthDepleted OnHealthDepleted;

	UFUNCTION(BlueprintPure)
	int32 GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure)
	int32 GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure)
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintCallable)
	void Heal(int32 Amount);

private:
	// Game uses small heart counts (e.g. 3); int32 is required for Blueprint exposure.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = true, ClampMin = "1"))
	int32 MaxHealth = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = true, ClampMin = "0"))
	int32 CurrentHealth = 3;

	bool bIsDead = false;

	UFUNCTION()
	void HandleOwnerTakeAnyDamage(
		AActor* DamagedActor,
		float Damage,
		const UDamageType* DamageType,
		AController* InstigatedBy,
		AActor* DamageCauser
	);

protected:
	virtual void BeginPlay() override;
};
