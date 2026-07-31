#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyAIComponent.generated.h"

class ACharacterBase;
class UCharacterStateComponent;
class AWaveEnemyManager;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class YOWYOW_API UEnemyAIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyAIComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(EditAnywhere, Category = "Enemy AI")
	float DetectionRange = 700.f;

	UPROPERTY(EditAnywhere, Category = "Enemy AI")
	float AttackRange = 130.f;

	UPROPERTY(EditAnywhere, Category = "Enemy AI")
	float MoveSpeed = 250.f;

	UPROPERTY(EditAnywhere, Category = "Enemy AI")
	float AttackCooldown = 1.5f;

	/**
	 * While horizontal speed is above this, skip chase/attack so knockback can play out.
	 * Otherwise AddActorWorldOffset every tick cancels LaunchCharacter.
	 */
	UPROPERTY(EditAnywhere, Category = "Enemy AI|Combat", meta = (ClampMin = "0.0"))
	float KnockbackIgnoreSpeed = 100.f;

	UPROPERTY(EditAnywhere, Category = "Enemy AI|Wave")
	AWaveEnemyManager* WaveManager = nullptr;

	UPROPERTY(EditAnywhere, Category = "Enemy AI|Wave")
	float TokenReleaseDelay = 0.8f;

	UPROPERTY()
	ACharacterBase* OwnerCharacter = nullptr;

	UPROPERTY()
	APawn* PlayerPawn = nullptr;

	UPROPERTY()
	UCharacterStateComponent* StateComponent = nullptr;

	FTimerHandle AttackTokenReleaseTimerHandle;

	float LastAttackTime = -999.f;

	void UpdateAI(float DeltaTime);
	bool CanAct() const;
	bool CanAttack() const;
	void ReleaseAttackToken();
};