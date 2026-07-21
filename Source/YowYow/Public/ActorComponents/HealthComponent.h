// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class AController;
class AActor;
class UHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FOnHealthChanged,
	UHealthComponent*, HealthComponent,
	int8, CurrentHealth,
	int8, MaxHealth,
	float, DeltaHealth
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(
	FOnHealthDamageTaken,
	UHealthComponent*, HealthComponent,
	int8, Damage,
	int8, CurrentHealth,
	AActor*, DamageCauser,
	AController*, Instigator
);

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
	// Sets default values for this component's properties
	UHealthComponent();

	UPROPERTY(BlueprintAssignable)
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FOnHealthDamageTaken OnHealthDamageTaken;

	UPROPERTY(BlueprintAssignable)
	FOnHealthDepleted OnHealthDepleted;

	UFUNCTION(BlueprintPure)
	int8 GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure)
	int8 GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure)
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintCallable)
	void Heal(int8 Amount);

private:
	// I don't think anyone will have over 127 health
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = true))
	int8 MaxHealth = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = true))
	int8 CurrentHealth = 3;

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
	// Called when the game starts
	virtual void BeginPlay() override;
};
