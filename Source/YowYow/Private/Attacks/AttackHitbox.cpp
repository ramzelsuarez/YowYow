#include "Attacks/AttackHitbox.h"

#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#endif

#include "Components/SceneComponent.h"
#include "Engine/World.h"

namespace AttackHitboxDefaults
{
	constexpr float MinimumRange = 1.f;
	constexpr float DefaultSpeed = 600.f;
	constexpr float TraceHeight = 50.f;
}

AAttackHitbox::AAttackHitbox()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);

	SetActorEnableCollision(false);
}

void AAttackHitbox::Initialize(
	AActor* InSourceActor,
	const FAttackData& InAttackData,
	float InHitboxRadius,
	USceneComponent* InAttachedSource
)
{
	SourceActor = InSourceActor;
	AttachedSource = InAttachedSource;
	bUseAttachedSource = AttachedSource.IsValid();
	AttackData = InAttackData;
	HitboxRadius = FMath::Max(InHitboxRadius, 1.f);

	if (!SourceActor.IsValid())
	{
		FinishAttack();
		return;
	}

	AttackForward = SourceActor->GetActorForwardVector().GetSafeNormal2D();
	AttackRight = SourceActor->GetActorRightVector().GetSafeNormal2D();
	ArcCenter = GetSourceLocation() + FVector::UpVector * AttackHitboxDefaults::TraceHeight;

	const float Range = FMath::Max(AttackData.Range, AttackHitboxDefaults::MinimumRange);
	const float Speed = AttackData.Speed > 0.f ? AttackData.Speed : AttackHitboxDefaults::DefaultSpeed;
	Duration = Range / Speed;

	const FVector InitialLocation = bUseAttachedSource
		? AttachedSource->GetComponentLocation()
		: ArcCenter + AttackRight * Range;

	SetActorLocation(InitialLocation);

#if WITH_EDITOR
	DrawDebugSphere(GetWorld(), InitialLocation, HitboxRadius, 16, FColor::Red, false, 0.f, 0, 1.5f);
#endif

	SetActorTickEnabled(true);
}

void AAttackHitbox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!SourceActor.IsValid())
	{
		FinishAttack();
		return;
	}

	if (bUseAttachedSource)
	{
		if (!AttachedSource.IsValid())
		{
			FinishAttack();
			return;
		}

		TickAttached(DeltaTime);
	}
	else
	{
		TickRound(DeltaTime);
	}
}

void AAttackHitbox::TickAttached(float DeltaTime)
{
	ElapsedTime += DeltaTime;
	MoveAndTrace(AttachedSource->GetComponentLocation(), true);

	if (ElapsedTime >= Duration)
	{
		FinishAttack();
	}
}

void AAttackHitbox::TickRound(float DeltaTime)
{
	const float Range = FMath::Max(AttackData.Range, AttackHitboxDefaults::MinimumRange);
	const float Speed = AttackData.Speed > 0.f ? AttackData.Speed : AttackHitboxDefaults::DefaultSpeed;
	const float AngularSpeed = FMath::RadiansToDegrees(Speed / Range);

	AttackForward = SourceActor->GetActorForwardVector().GetSafeNormal2D();
	AttackRight = SourceActor->GetActorRightVector().GetSafeNormal2D();
	ArcCenter = GetSourceLocation() + FVector::UpVector * AttackHitboxDefaults::TraceHeight;
	CurrentArcAngle = FMath::Max(CurrentArcAngle - AngularSpeed * DeltaTime, -90.f);
	const float AngleRadians = FMath::DegreesToRadians(CurrentArcAngle);
	const FVector ArcDirection = AttackForward * FMath::Cos(AngleRadians) + AttackRight * FMath::Sin(AngleRadians);

	MoveAndTrace(ArcCenter + ArcDirection * Range, true);

	if (CurrentArcAngle <= -90.f)
	{
		FinishAttack();
	}
}

FVector AAttackHitbox::GetSourceLocation() const
{
	if (const USceneComponent* SourceRoot = SourceActor.IsValid() ? SourceActor->GetRootComponent() : nullptr)
	{
		return SourceRoot->GetComponentLocation();
	}

	return SourceActor.IsValid() ? SourceActor->GetActorLocation() : FVector::ZeroVector;
}

void AAttackHitbox::MoveAndTrace(const FVector& NewLocation, bool bShouldTrace)
{
	const FVector PreviousLocation = GetActorLocation();
	SetActorLocation(NewLocation);

#if WITH_EDITOR
	const FColor DebugColor = bShouldTrace ? FColor::Red : FColor::Yellow;
	DrawDebugSphere(GetWorld(), NewLocation, HitboxRadius, 16, DebugColor, false, 0.f, 0, 1.5f);
	DrawDebugLine(GetWorld(), PreviousLocation, NewLocation, DebugColor, false, 0.f, 0, 1.5f);
#endif

	if (bShouldTrace)
	{
		TraceHits(PreviousLocation, NewLocation);
	}
}

void AAttackHitbox::TraceHits(const FVector& Start, const FVector& End)
{
	if (!GetWorld() || !SourceActor.IsValid())
	{
		return;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AttackHitbox), false, SourceActor.Get());
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	TArray<FHitResult> Hits;
	GetWorld()->SweepMultiByObjectType(
		Hits,
		Start,
		End,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(HitboxRadius),
		QueryParams
	);

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!IsValid(HitActor) || HitActor == SourceActor.Get() || HitActors.Contains(HitActor))
		{
			continue;
		}

		HitActors.Add(HitActor);
		HandleHit(HitActor);
	}
}

void AAttackHitbox::HandleHit(AActor* HitActor)
{
	// Intentionally left as a no-op until the damage receiving contract is defined.
}

void AAttackHitbox::FinishAttack()
{
	if (bFinished)
	{
		return;
	}

	bFinished = true;
	SetActorTickEnabled(false);
	OnFinished.Broadcast(this);
	Destroy();
}
