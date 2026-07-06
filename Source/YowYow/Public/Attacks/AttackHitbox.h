#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/AttackTypes.h"
#include "AttackHitbox.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FAttackHitboxFinished, class AAttackHitbox*);

class USceneComponent;

UCLASS(NotBlueprintable)
class YOWYOW_API AAttackHitbox : public AActor
{
	GENERATED_BODY()

public:
	AAttackHitbox();

	void Initialize(
		AActor* InSourceActor,
		const FAttackData& InAttackData,
		float InHitboxRadius,
		USceneComponent* InAttachedSource = nullptr
	);

	FAttackHitboxFinished OnFinished;

protected:
	virtual void Tick(float DeltaTime) override;

private:
	void TickAttached(float DeltaTime);
	void TickRound(float DeltaTime);
	FVector GetSourceLocation() const;
	void MoveAndTrace(const FVector& NewLocation, bool bShouldTrace);
	void TraceHits(const FVector& Start, const FVector& End);
	void HandleHit(AActor* HitActor);
	void FinishAttack();

	UPROPERTY()
	USceneComponent* SceneRoot = nullptr;

	TWeakObjectPtr<AActor> SourceActor;
	TWeakObjectPtr<USceneComponent> AttachedSource;
	FAttackData AttackData;
	FVector AttackForward = FVector::ForwardVector;
	FVector AttackRight = FVector::RightVector;
	FVector ArcCenter = FVector::ZeroVector;
	TSet<TWeakObjectPtr<AActor>> HitActors;

	float HitboxRadius = 32.f;
	float CurrentArcAngle = 90.f;
	float ElapsedTime = 0.f;
	float Duration = 0.f;
	bool bUseAttachedSource = false;
	bool bFinished = false;
};
