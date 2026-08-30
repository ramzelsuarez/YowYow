#include "Attacks/AttackHitbox.h"

#include "Combat/CombatImpactLibrary.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

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

#if !WITH_EDITOR
	bDrawDebug = false;
#endif
}

void AAttackHitbox::Initialize(
	AActor* InSourceActor,
	const FAttackData& InAttackData,
	USceneComponent* InAttachedSource,
	float InOrbitSideSign
)
{
	SourceActor = InSourceActor;
	AttachedSource = InAttachedSource;
	AttackData = InAttackData;
	Motion = InAttackData.Motion;
	HitboxRadius = FMath::Max(InAttackData.HitboxRadius > 0.f ? InAttackData.HitboxRadius : 32.f, 1.f);
	OrbitSideSign = FMath::IsNearlyZero(InOrbitSideSign) ? -1.f : FMath::Sign(InOrbitSideSign);

	if (!SourceActor.IsValid())
	{
		FinishAttack();
		return;
	}

	if (Motion == EAttackMotion::FollowSource && !AttachedSource.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("AttackHitbox FollowSource requires an attached source on %s"),
			*GetNameSafe(InSourceActor));
		FinishAttack();
		return;
	}

	AttackForward = SourceActor->GetActorForwardVector().GetSafeNormal2D();
	AttackRight = SourceActor->GetActorRightVector().GetSafeNormal2D();
	ArcCenter = GetSourceLocation() + FVector::UpVector * AttackHitboxDefaults::TraceHeight;

	const float Range = FMath::Max(AttackData.Range, AttackHitboxDefaults::MinimumRange);
	const float Speed = AttackData.Speed > 0.f ? AttackData.Speed : AttackHitboxDefaults::DefaultSpeed;
	Duration = Range / Speed;

	CurrentArcAngle = AttackData.ArcStartDegrees;
	// Crescent starts behind the attacker (180°) and sweeps 180° to the front once.
	OrbitAngleDegrees = 180.f;
	OrbitTravelDegrees = 0.f;
	ElapsedTime = 0.f;
	PeakAlongForward = 0.f;
	bOutboundArmed = false;

	FVector InitialLocation = ArcCenter;
	switch (Motion)
	{
	case EAttackMotion::FollowSource:
		InitialLocation = AttachedSource->GetComponentLocation();
		break;
	case EAttackMotion::OrbitCircle:
		// Back of the character.
		InitialLocation = ArcCenter - AttackForward * Range;
		break;
	case EAttackMotion::ArcSweep:
	default:
		{
			const float AngleRadians = FMath::DegreesToRadians(CurrentArcAngle);
			const FVector ArcDirection =
				AttackForward * FMath::Cos(AngleRadians) + AttackRight * FMath::Sin(AngleRadians);
			InitialLocation = ArcCenter + ArcDirection * Range;
		}
		break;
	}

	SetActorLocation(InitialLocation);
	DrawDebugAt(InitialLocation, InitialLocation, false);
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

	switch (Motion)
	{
	case EAttackMotion::FollowSource:
		TickFollowSource(DeltaTime);
		break;
	case EAttackMotion::OrbitCircle:
		TickOrbitCircle(DeltaTime);
		break;
	case EAttackMotion::ArcSweep:
	default:
		TickArcSweep(DeltaTime);
		break;
	}
}

void AAttackHitbox::TickFollowSource(float DeltaTime)
{
	if (!AttachedSource.IsValid())
	{
		FinishAttack();
		return;
	}

	ElapsedTime += DeltaTime;

	const FVector SourceLoc = AttachedSource->GetComponentLocation();
	const float AlongForward = FVector::DotProduct(SourceLoc - GetSourceLocation(), AttackForward);

	if (AlongForward > PeakAlongForward + 1.f)
	{
		PeakAlongForward = AlongForward;
		bOutboundArmed = true;
	}

	const bool bStartedReturn = bOutboundArmed && AlongForward + 2.f < PeakAlongForward;
	const bool bFullPath = AttackData.FollowDamageWindow == EFollowSourceDamageWindow::FullPath;
	const bool bShouldTrace = bFullPath || !bStartedReturn;

	MoveAndTrace(SourceLoc, bShouldTrace);

	// OutboundOnly: live until the yoyo turns around at the triangle apex.
	// Safety cap in case presentation never returns (lost source, etc.).
	const float SafetyDuration = FMath::Max(Duration * 4.f, Duration + 0.35f);
	if (ElapsedTime >= SafetyDuration)
	{
		FinishAttack();
		return;
	}

	if (bFullPath)
	{
		if (bStartedReturn && AlongForward <= 15.f)
		{
			FinishAttack();
		}
		return;
	}

	if (bStartedReturn)
	{
		FinishAttack();
	}
}

void AAttackHitbox::TickArcSweep(float DeltaTime)
{
	const float Range = FMath::Max(AttackData.Range, AttackHitboxDefaults::MinimumRange);
	const float Speed = AttackData.Speed > 0.f ? AttackData.Speed : AttackHitboxDefaults::DefaultSpeed;
	const float AngularSpeed = FMath::RadiansToDegrees(Speed / Range);

	AttackForward = SourceActor->GetActorForwardVector().GetSafeNormal2D();
	AttackRight = SourceActor->GetActorRightVector().GetSafeNormal2D();
	ArcCenter = GetSourceLocation() + FVector::UpVector * AttackHitboxDefaults::TraceHeight;

	const float EndAngle = AttackData.ArcEndDegrees;
	const float StartAngle = AttackData.ArcStartDegrees;
	const bool bDecreasing = EndAngle < StartAngle;

	if (bDecreasing)
	{
		CurrentArcAngle = FMath::Max(CurrentArcAngle - AngularSpeed * DeltaTime, EndAngle);
	}
	else
	{
		CurrentArcAngle = FMath::Min(CurrentArcAngle + AngularSpeed * DeltaTime, EndAngle);
	}

	const float AngleRadians = FMath::DegreesToRadians(CurrentArcAngle);
	const FVector ArcDirection =
		AttackForward * FMath::Cos(AngleRadians) + AttackRight * FMath::Sin(AngleRadians);

	MoveAndTrace(ArcCenter + ArcDirection * Range, true);

	const bool bReachedEnd = bDecreasing
		? CurrentArcAngle <= EndAngle
		: CurrentArcAngle >= EndAngle;

	if (bReachedEnd)
	{
		FinishAttack();
	}
}

void AAttackHitbox::TickOrbitCircle(float DeltaTime)
{
	// Dual medialuna: start at back (180°), sweep once to front (0°/360°).
	// SideSign -1 → through right (180→90→0). SideSign +1 → through left (180→270→360).
	constexpr float CrescentTravelDegrees = 180.f;

	const float Range = FMath::Max(AttackData.Range, AttackHitboxDefaults::MinimumRange);
	const float Speed = AttackData.Speed > 0.f ? AttackData.Speed : AttackHitboxDefaults::DefaultSpeed;
	const float AngularSpeed = FMath::RadiansToDegrees(Speed / Range);

	AttackForward = SourceActor->GetActorForwardVector().GetSafeNormal2D();
	AttackRight = SourceActor->GetActorRightVector().GetSafeNormal2D();
	ArcCenter = GetSourceLocation() + FVector::UpVector * AttackHitboxDefaults::TraceHeight;

	OrbitTravelDegrees = FMath::Min(OrbitTravelDegrees + AngularSpeed * DeltaTime, CrescentTravelDegrees);
	OrbitAngleDegrees = 180.f + OrbitSideSign * OrbitTravelDegrees;

	const float AngleRadians = FMath::DegreesToRadians(OrbitAngleDegrees);
	const FVector OrbitDirection =
		AttackForward * FMath::Cos(AngleRadians) + AttackRight * FMath::Sin(AngleRadians);

	MoveAndTrace(ArcCenter + OrbitDirection * Range, true);

	// One crescent only — stop at the front, never full loops.
	if (OrbitTravelDegrees >= CrescentTravelDegrees)
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
	DrawDebugAt(NewLocation, PreviousLocation, bShouldTrace);

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
	if (!IsValid(HitActor) || !SourceActor.IsValid())
	{
		return;
	}

	APawn* InstigatorPawn = Cast<APawn>(SourceActor.Get());
	AController* InstigatorController = InstigatorPawn ? InstigatorPawn->GetController() : nullptr;

	UGameplayStatics::ApplyDamage(
		HitActor,
		AttackData.Damage,
		InstigatorController,
		SourceActor.Get(),
		nullptr
	);

	// Away from attacker (horizontal). Works for player hits on enemies and vice versa.
	FVector KnockbackDir = HitActor->GetActorLocation() - SourceActor->GetActorLocation();
	KnockbackDir.Z = 0.f;
	if (KnockbackDir.IsNearlyZero())
	{
		KnockbackDir = SourceActor->GetActorForwardVector().GetSafeNormal2D();
	}

	UCombatImpactLibrary::ApplyKnockback(
		HitActor,
		KnockbackDir,
		AttackData.Knockback,
		/*bHorizontalOnly=*/true,
		/*bOverrideXY=*/true,
		/*bOverrideZ=*/false
	);
	UCombatImpactLibrary::ApplyHitStopPair(
		SourceActor.Get(),
		HitActor,
		AttackData.HitStopDuration,
		AttackData.HitStopDilation
	);
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

void AAttackHitbox::DrawDebugAt(const FVector& Location, const FVector& PreviousLocation, bool bShouldTrace) const
{
	if (!bDrawDebug || !GetWorld())
	{
		return;
	}

	const FColor DebugColor = bShouldTrace ? FColor::Red : FColor::Yellow;
	DrawDebugSphere(GetWorld(), Location, HitboxRadius, 16, DebugColor, false, 0.f, 0, 1.5f);
	DrawDebugLine(GetWorld(), PreviousLocation, Location, DebugColor, false, 0.f, 0, 1.5f);
}
