#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyAIComponent.generated.h"

class ACharacterBase;
class UCharacterStateComponent;

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

	UPROPERTY(EditAnywhere, Category = "Enemy AI")
	bool bIgnoreZ = true;

	UPROPERTY()
	ACharacterBase* OwnerCharacter = nullptr;

	UPROPERTY()
	APawn* PlayerPawn = nullptr;

	UPROPERTY()
	UCharacterStateComponent* StateComponent = nullptr;

	float LastAttackTime = -999.f;

	void UpdateAI(float DeltaTime);
	bool CanAct() const;
	bool CanAttack() const;
};