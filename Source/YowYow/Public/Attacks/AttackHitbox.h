#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/AttackTypes.h"
#include "AttackHitbox.generated.h"

/** Consumed by UAttackComponent (attack lifecycle / combo). */
DECLARE_MULTICAST_DELEGATE_OneParam(FAttackHitboxFinished, class AAttackHitbox*);

class USceneComponent;

UCLASS(NotBlueprintable)
class YOWYOW_API AAttackHitbox : public AActor
{
	GENERATED_BODY()

public:
	AAttackHitbox();

	/**
	 * @param InAttachedSource Required when AttackData.Motion == FollowSource.
	 */
	void Initialize(
		AActor* InSourceActor,
		const FAttackData& InAttackData,
		USceneComponent* InAttachedSource = nullptr
	);

	FAttackHitboxFinished OnFinished;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawDebug = false;

protected:
	virtual void Tick(float DeltaTime) override;

private:
	void TickFollowSource(float DeltaTime);
	void TickArcSweep(float DeltaTime);
	void TickOrbitCircle(float DeltaTime);
	FVector GetSourceLocation() const;
	void MoveAndTrace(const FVector& NewLocation, bool bShouldTrace);
	void TraceHits(const FVector& Start, const FVector& End);
	void HandleHit(AActor* HitActor);
	void FinishAttack();
	void DrawDebugAt(const FVector& Location, const FVector& PreviousLocation, bool bShouldTrace) const;

	UPROPERTY()
	USceneComponent* SceneRoot = nullptr;

	TWeakObjectPtr<AActor> SourceActor;
	TWeakObjectPtr<USceneComponent> AttachedSource;
	FAttackData AttackData;
	EAttackMotion Motion = EAttackMotion::ArcSweep;
	FVector AttackForward = FVector::ForwardVector;
	FVector AttackRight = FVector::RightVector;
	FVector ArcCenter = FVector::ZeroVector;
	TSet<TWeakObjectPtr<AActor>> HitActors;

	float HitboxRadius = 32.f;
	float CurrentArcAngle = 90.f;
	float OrbitAngleDegrees = 0.f;
	float ElapsedTime = 0.f;
	float Duration = 0.f;
	bool bFinished = false;
};
