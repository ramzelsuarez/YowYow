#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/AttackTypes.h"
#include "AttackHitbox.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FAttackHitboxFinished, class AAttackHitbox*);

UCLASS(NotBlueprintable)
class YOWYOW_API AAttackHitbox : public AActor
{
	GENERATED_BODY()

public:
	AAttackHitbox();

	void Initialize(AActor* InSourceActor, const FAttackData& InAttackData, float InHitboxRadius);

	FAttackHitboxFinished OnFinished;

protected:
	virtual void Tick(float DeltaTime) override;

private:
	void TickRound(float DeltaTime);
	void MoveAndTrace(const FVector& NewLocation, bool bShouldTrace);
	void TraceHits(const FVector& Start, const FVector& End);
	void HandleHit(AActor* HitActor);
	void FinishAttack();

	TWeakObjectPtr<AActor> SourceActor;
	FAttackData AttackData;
	FVector AttackForward = FVector::ForwardVector;
	FVector AttackRight = FVector::RightVector;
	FVector ArcCenter = FVector::ZeroVector;
	TSet<TWeakObjectPtr<AActor>> HitActors;

	float HitboxRadius = 32.f;
	float CurrentArcAngle = 90.f;
	bool bFinished = false;
};
