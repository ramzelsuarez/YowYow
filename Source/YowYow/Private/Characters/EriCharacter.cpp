// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/EriCharacter.h"

#include "ActorComponents/AttackComponent.h"
#include "ActorComponents/CharacterStateComponent.h"
#include "ActorComponents/HomingAttackComponent.h"
#include "ActorComponents/TrickGaugeComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/StaticMesh.h"
#include "Interfaces/Homingable.h"
#include "CharacterStates/HomingStates.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "PaperFlipbookComponent.h"

AEriCharacter::AEriCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 500.0f;
	CameraBoom->bUsePawnControlRotation = true;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);

	// Components only — mesh + rest transform come from the BP/viewport (never forced in BeginPlay).
	YoYoRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("YoYoRight"));
	YoYoRight->SetupAttachment(GetRootComponent());
	YoYoRight->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	YoYoLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("YoYoLeft"));
	YoYoLeft->SetupAttachment(GetRootComponent());
	YoYoLeft->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	YoYoRightVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("YoYoRightVFX"));
	YoYoRightVFX->SetupAttachment(YoYoRight);
	YoYoRightVFX->SetAutoActivate(false);

	YoYoLeftVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("YoYoLeftVFX"));
	YoYoLeftVFX->SetupAttachment(YoYoLeft);
	YoYoLeftVFX->SetAutoActivate(false);
	
	// Trick Aura VFX
	TrickAuraVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrickAuraVFX"));
	TrickAuraVFX->SetupAttachment(GetRootComponent());
	TrickAuraVFX->SetAutoActivate(false);
}

void AEriCharacter::BeginPlay()
{
	Super::BeginPlay();

	HomingAttackComponent = FindComponentByClass<UHomingAttackComponent>();
	TrickGaugeComponent = FindComponentByClass<UTrickGaugeComponent>();

	ApplyYoYoMeshAssets();
	CacheYoYoRests();
	AttachYoYosToHandSockets();

	if (AttackComponent)
	{
		AttackComponent->SetHandSources(YoYoRight, YoYoLeft);
		AttackComponent->SetRequiresPresentationComplete(true);
		AttackComponent->OnAttackStarted.AddDynamic(this, &AEriCharacter::HandleAttackStarted);
		AttackComponent->OnAttackFinished.AddDynamic(this, &AEriCharacter::HandleAttackFinished);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no AttackComponent on the Blueprint."), *GetName());
	}

	if (HomingAttackComponent)
	{
		HomingAttackComponent->OnHomingAttackFinished.AddDynamic(this, &AEriCharacter::HandleHomingAttackFinished);
	}
	
	if (TrickAuraVFXSystem && TrickAuraVFX)
	{
		TrickAuraVFX->SetAsset(TrickAuraVFXSystem);
	}
}

void AEriCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SetHomingCameraLocked(false);

	if (AttackComponent)
	{
		AttackComponent->OnAttackStarted.RemoveDynamic(this, &AEriCharacter::HandleAttackStarted);
		AttackComponent->OnAttackFinished.RemoveDynamic(this, &AEriCharacter::HandleAttackFinished);
	}

	if (HomingAttackComponent)
	{
		HomingAttackComponent->OnHomingAttackFinished.RemoveDynamic(this, &AEriCharacter::HandleHomingAttackFinished);
	}
	
	StopYoYoAttackVFX();
	ClearPresentationAttackType();
	
	// for trick VFX
	if (TrickAuraVFX)
	{
		TrickAuraVFX->DeactivateImmediate();
	}
	
	Super::EndPlay(EndPlayReason);
}

void AEriCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (PresentationMode == EYoYoPresentationMode::None)
	{
		AttachYoYosToHandSockets();
	}
	UpdateYoYoPresentation(DeltaTime);
	UpdateHomingCameraLock(DeltaTime);
}

void AEriCharacter::ApplyYoYoMeshAssets()
{
	// Only mesh assets — do NOT touch relative location (viewport/BP owns rest pose).
	if (YoYoRight && YoYoMeshAsset)
	{
		YoYoRight->SetStaticMesh(YoYoMeshAsset);
	}

	if (YoYoLeft)
	{
		UStaticMesh* LeftMesh = YoYoLeftMeshAsset ? YoYoLeftMeshAsset : YoYoMeshAsset;
		if (LeftMesh)
		{
			YoYoLeft->SetStaticMesh(LeftMesh);
		}
	}
}

void AEriCharacter::CacheYoYoRests()
{
	// Viewport rest is fallback if a sprite frame has no socket. Cache before attaching.
	RightHand.Component = YoYoRight;
	RightHand.SocketName = YoYoRightSocketName;
	LeftHand.Component = YoYoLeft;
	LeftHand.SocketName = YoYoLeftSocketName;

	if (YoYoRight)
	{
		RightHand.RestRelative = YoYoRight->GetRelativeLocation();
		RightHand.RestRelativeRotation = YoYoRight->GetRelativeRotation();
	}
	if (YoYoLeft)
	{
		LeftHand.RestRelative = YoYoLeft->GetRelativeLocation();
		LeftHand.RestRelativeRotation = YoYoLeft->GetRelativeRotation();
	}
}

void AEriCharacter::AttachYoYosToHandSockets()
{
	AttachYoYoToHandSocket(
		YoYoRight, RightHand.SocketName, RightHand.RestRelative, RightHand.RestRelativeRotation);
	AttachYoYoToHandSocket(
		YoYoLeft, LeftHand.SocketName, LeftHand.RestRelative, LeftHand.RestRelativeRotation);
}

void AEriCharacter::AttachYoYoToHandSocket(
	UStaticMeshComponent* YoYo,
	FName SocketName,
	const FVector& FallbackRelative,
	const FRotator& RestRotation)
{
	if (!YoYo)
	{
		return;
	}

	UPaperFlipbookComponent* Flipbook = GetSprite();
	if (Flipbook && !SocketName.IsNone() && Flipbook->DoesSocketExist(SocketName))
	{
		if (YoYo->GetAttachParent() != Flipbook || YoYo->GetAttachSocketName() != SocketName)
		{
			// Snap location to the hand; keep the mesh rotation authored in the BP.
			const FAttachmentTransformRules AttachRules(
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::KeepRelative,
				EAttachmentRule::KeepWorld,
				false
			);
			YoYo->AttachToComponent(Flipbook, AttachRules, SocketName);
		}
		YoYo->SetRelativeRotation(RestRotation);
		return;
	}

	if (YoYo->GetAttachParent() != GetRootComponent() || !YoYo->GetAttachSocketName().IsNone())
	{
		YoYo->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		YoYo->SetRelativeLocation(FallbackRelative);
		YoYo->SetRelativeRotation(RestRotation);
	}
}

void AEriCharacter::DetachYoYoForFlight(USceneComponent* YoYo)
{
	if (!YoYo || !YoYo->GetAttachParent())
	{
		return;
	}

	YoYo->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
}

void AEriCharacter::GatherHands(EYoYoHand Hand, TArray<FYoYoRuntime*>& OutHands)
{
	OutHands.Reset();
	switch (Hand)
	{
	case EYoYoHand::Right:
		if (RightHand.Component.IsValid())
		{
			OutHands.Add(&RightHand);
		}
		break;
	case EYoYoHand::Left:
		if (LeftHand.Component.IsValid())
		{
			OutHands.Add(&LeftHand);
		}
		break;
	case EYoYoHand::Both:
		if (RightHand.Component.IsValid())
		{
			OutHands.Add(&RightHand);
		}
		if (LeftHand.Component.IsValid())
		{
			OutHands.Add(&LeftHand);
		}
		break;
	}
}

FVector AEriCharacter::GetRestWorldLocation(const FYoYoRuntime& Hand) const
{
	if (UPaperFlipbookComponent* Flipbook = GetSprite())
	{
		if (!Hand.SocketName.IsNone() && Flipbook->DoesSocketExist(Hand.SocketName))
		{
			return Flipbook->GetSocketLocation(Hand.SocketName);
		}
	}

	return GetActorTransform().TransformPosition(Hand.RestRelative);
}

void AEriCharacter::HandleAttackStarted(EAttackType AttackType, FAttackData StartedAttackData)
{
	switch (AttackType)
	{
	case EAttackType::Normal:
		CurrentYoYoAttackVFX = NormalAttackVFXSystem;
		break;

	case EAttackType::Area:
		CurrentYoYoAttackVFX = AreaAttackVFXSystem;
		break;

	default:
		CurrentYoYoAttackVFX = nullptr;
		break;
	}

	PresentationAttackType = AttackType;
	BeginYoYoPresentation(StartedAttackData);
}

void AEriCharacter::HandleAttackFinished(EAttackType AttackType, bool bCompleted)
{
	if (PresentationMode == EYoYoPresentationMode::None)
	{
		if (AttackComponent)
		{
			AttackComponent->NotifyPresentationComplete();
		}
		return;
	}

	// Thrust owns its path: active yoyos reach the apex before returning.
	// Orbit still returns when the crescent hitboxes close.
	if (PresentationMode == EYoYoPresentationMode::Orbit)
	{
		StartYoYoReturn();
	}
}

void AEriCharacter::BeginYoYoPresentation(const FAttackData& InAttackData)
{
	// Rest pose is cached once in BeginPlay — never re-read mid-flight (that corrupts home).
	bYoYoReturning = false;
	OrbitTravelDegrees = 0.f;
	RightHand.bActive = false;
	LeftHand.bActive = false;

	CachedAttackForward = GetActorForwardVector().GetSafeNormal2D();
	if (CachedAttackForward.IsNearlyZero())
	{
		CachedAttackForward = FVector::ForwardVector;
	}

	YoYoCurrentSpeed = InAttackData.Speed > 0.f ? InAttackData.Speed : 600.f;
	OrbitRadius = FMath::Max(InAttackData.Range, 1.f);

	TArray<FYoYoRuntime*> Hands;
	GatherHands(InAttackData.YoYoHand, Hands);

	if (Hands.IsEmpty())
	{
		PresentationMode = EYoYoPresentationMode::None;
		if (AttackComponent)
		{
			AttackComponent->NotifyPresentationComplete();
		}
		return;
	}

	AttackYoYoHand = InAttackData.YoYoHand;

	for (FYoYoRuntime* HandRuntime : Hands)
	{
		if (HandRuntime)
		{
			DetachYoYoForFlight(HandRuntime->Component.Get());
		}
	}

	if (InAttackData.Motion == EAttackMotion::OrbitCircle)
	{
		// Dual medialunas: selected hands start behind, sweep once to the front.
		PresentationMode = EYoYoPresentationMode::Orbit;
		OrbitTravelDegrees = 0.f;

		if (RightHand.Component.IsValid() && Hands.Contains(&RightHand))
		{
			RightHand.bActive = true;
			RightHand.OrbitSideSign = -1.f;
		}
		if (LeftHand.Component.IsValid() && Hands.Contains(&LeftHand))
		{
			LeftHand.bActive = true;
			LeftHand.OrbitSideSign = +1.f;
		}

		if (!RightHand.bActive && !LeftHand.bActive)
		{
			for (int32 Index = 0; Index < Hands.Num(); ++Index)
			{
				Hands[Index]->bActive = true;
				Hands[Index]->OrbitSideSign = (Index == 0) ? -1.f : +1.f;
			}
		}

		StartYoYoAttackVFX();
		return;
	}

	// Thrust: one hand flies straight out; both meet at a shared apex (triangle).
	PresentationMode = EYoYoPresentationMode::Thrust;
	const float Range = FMath::Max(InAttackData.Range, 1.f);
	const FVector ActorLoc = GetActorLocation();
	const bool bBothHands = Hands.Num() > 1;

	FVector SharedApex = ActorLoc + CachedAttackForward * Range;
	if (bBothHands)
	{
		float ApexZ = 0.f;
		int32 ApexZCount = 0;
		for (const FYoYoRuntime* HandRuntime : Hands)
		{
			ApexZ += GetRestWorldLocation(*HandRuntime).Z;
			++ApexZCount;
		}
		SharedApex.Z = (ApexZCount > 0) ? (ApexZ / ApexZCount) : ActorLoc.Z;
	}

	float MaxDist = 0.f;
	for (FYoYoRuntime* HandRuntime : Hands)
	{
		HandRuntime->bActive = true;
		USceneComponent* Comp = HandRuntime->Component.Get();
		if (!Comp)
		{
			continue;
		}

		const FVector StartWorld = Comp->GetComponentLocation();
		HandRuntime->PathStartWorld = StartWorld;
		HandRuntime->OutboundWorld = bBothHands
			? SharedApex
			: StartWorld + CachedAttackForward * Range;
		MaxDist = FMath::Max(MaxDist, FVector::Dist(StartWorld, HandRuntime->OutboundWorld));
	}

	StartYoYoAttackVFX();

	ThrustElapsed = 0.f;
	ThrustDuration = (YoYoCurrentSpeed > 0.f) ? (MaxDist / YoYoCurrentSpeed) : 0.f;
	if (ThrustDuration <= KINDA_SMALL_NUMBER)
	{
		StartYoYoReturn();
	}
}

void AEriCharacter::BeginHomingYoyoCharge()
{
	ClearPresentationAttackType();

	if (PresentationMode != EYoYoPresentationMode::None && PresentationMode != EYoYoPresentationMode::Homing)
	{
		FinishYoYoPresentation();
	}

	PresentationMode = EYoYoPresentationMode::Homing;
	bYoYoReturning = false;
	RightHand.bActive = false;
	LeftHand.bActive = false;

	TArray<FYoYoRuntime*> Hands;
	GatherHands(EYoYoHand::Both, Hands);
	for (FYoYoRuntime* HandRuntime : Hands)
	{
		if (!HandRuntime)
		{
			continue;
		}

		DetachYoYoForFlight(HandRuntime->Component.Get());
		HandRuntime->bActive = true;
		if (USceneComponent* Comp = HandRuntime->Component.Get())
		{
			HandRuntime->PathStartWorld = Comp->GetComponentLocation();
		}
	}

	if (HomingAttackComponent)
	{
		YoYoCurrentSpeed = HomingAttackComponent->GetHomingSpeed();
	}
}

void AEriCharacter::UpdateHomingYoyos(float DeltaTime)
{
	if (bYoYoReturning)
	{
		bool bAnyMoving = false;
		auto ReturnHand = [&](FYoYoRuntime& Hand)
		{
			if (!Hand.bActive || !Hand.Component.IsValid())
			{
				return;
			}

			const FVector Target = GetRestWorldLocation(Hand);
			const float Speed = YoYoCurrentSpeed * YoYoReturnSpeedMultiplier;
			const FVector NewLoc = FMath::VInterpConstantTo(
				Hand.Component->GetComponentLocation(), Target, DeltaTime, Speed);
			Hand.Component->SetWorldLocation(NewLoc);
			if (!NewLoc.Equals(Target, 1.5f))
			{
				bAnyMoving = true;
			}
		};

		ReturnHand(RightHand);
		ReturnHand(LeftHand);
		if (!bAnyMoving)
		{
			FinishYoYoPresentation();
		}
		return;
	}

	FVector TargetLocation = GetActorLocation();
	if (HomingAttackComponent)
	{
		if (AActor* Target = HomingAttackComponent->GetCurrentHomingTarget())
		{
			TargetLocation = Target->Implements<UHomingable>()
				? IHomingable::Execute_GetTargetLocation(Target)
				: Target->GetActorLocation();
		}
	}

	const float ArriveDistance = HomingAttackComponent
		? FMath::Max(HomingAttackComponent->GetHitDistance(), 40.f)
		: 40.f;
	const float Speed = YoYoCurrentSpeed > 0.f ? YoYoCurrentSpeed : 600.f;

	bool bAllArrived = true;
	auto FlyHand = [&](FYoYoRuntime& Hand)
	{
		if (!Hand.bActive || !Hand.Component.IsValid())
		{
			return;
		}

		const FVector NewLoc = FMath::VInterpConstantTo(
			Hand.Component->GetComponentLocation(), TargetLocation, DeltaTime, Speed);
		Hand.Component->SetWorldLocation(NewLoc);
		if (FVector::DistSquared(NewLoc, TargetLocation) > FMath::Square(ArriveDistance))
		{
			bAllArrived = false;
		}
	};

	FlyHand(RightHand);
	FlyHand(LeftHand);

	if (bAllArrived
		&& HomingAttackComponent
		&& HomingAttackComponent->GetHomingState() == EHomingState::Charging)
	{
		HomingAttackComponent->BeginLaunch();
	}
}

void AEriCharacter::StartYoYoReturn()
{
	if (PresentationMode == EYoYoPresentationMode::None)
	{
		return;
	}

	bYoYoReturning = true;
}

bool AEriCharacter::AreActiveYoYosAtTarget(bool bReturning) const
{
	auto CheckHand = [&](const FYoYoRuntime& Hand) -> bool
	{
		if (!Hand.bActive || !Hand.Component.IsValid())
		{
			return true;
		}
		const FVector Target = bReturning ? GetRestWorldLocation(Hand) : Hand.OutboundWorld;
		return Hand.Component->GetComponentLocation().Equals(Target, 1.5f);
	};

	return CheckHand(RightHand) && CheckHand(LeftHand);
}

void AEriCharacter::UpdateYoYoPresentation(float DeltaTime)
{
	if (PresentationMode == EYoYoPresentationMode::None)
	{
		return;
	}

	if (PresentationMode == EYoYoPresentationMode::Homing)
	{
		UpdateHomingYoyos(DeltaTime);
		return;
	}

	if (PresentationMode == EYoYoPresentationMode::Orbit)
	{
		if (bYoYoReturning)
		{
			// Lerp both hands back to rest after the crescent pass.
			bool bAnyMoving = false;
			auto ReturnHand = [&](FYoYoRuntime& Hand)
			{
				if (!Hand.bActive || !Hand.Component.IsValid())
				{
					return;
				}
				const FVector Target = GetRestWorldLocation(Hand);
				const float Speed = YoYoCurrentSpeed * YoYoReturnSpeedMultiplier;
				const FVector NewLoc = FMath::VInterpConstantTo(
					Hand.Component->GetComponentLocation(), Target, DeltaTime, Speed);
				Hand.Component->SetWorldLocation(NewLoc);
				if (!NewLoc.Equals(Target, 1.5f))
				{
					bAnyMoving = true;
				}
			};
			ReturnHand(RightHand);
			ReturnHand(LeftHand);
			if (!bAnyMoving)
			{
				FinishYoYoPresentation();
			}
			return;
		}

		// Match hitbox: 180° travel from back to front, one medialuna per hand.
		constexpr float CrescentTravelDegrees = 180.f;
		const float AngularSpeed = FMath::RadiansToDegrees(YoYoCurrentSpeed / FMath::Max(OrbitRadius, 1.f));
		OrbitTravelDegrees = FMath::Min(OrbitTravelDegrees + AngularSpeed * DeltaTime, CrescentTravelDegrees);

		const FVector Center = GetActorLocation() + FVector::UpVector * 50.f;
		const FVector Forward = CachedAttackForward;
		const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();

		auto UpdateCrescentHand = [&](FYoYoRuntime& Hand)
		{
			if (!Hand.bActive || !Hand.Component.IsValid())
			{
				return;
			}
			const float AngleDeg = 180.f + Hand.OrbitSideSign * OrbitTravelDegrees;
			const float AngleRad = FMath::DegreesToRadians(AngleDeg);
			const FVector Dir = Forward * FMath::Cos(AngleRad) + Right * FMath::Sin(AngleRad);
			Hand.Component->SetWorldLocation(Center + Dir * OrbitRadius);
		};

		UpdateCrescentHand(RightHand);
		UpdateCrescentHand(LeftHand);

		if (OrbitTravelDegrees >= CrescentTravelDegrees)
		{
			StartYoYoReturn();
		}
		return;
	}

	// Thrust: shared alpha so both yoyos meet at the apex at the same time.
	ThrustElapsed += DeltaTime;

	const float Duration = bYoYoReturning
		? FMath::Max(ThrustDuration / FMath::Max(YoYoReturnSpeedMultiplier, 0.1f), KINDA_SMALL_NUMBER)
		: FMath::Max(ThrustDuration, KINDA_SMALL_NUMBER);
	const float Alpha = FMath::Clamp(ThrustElapsed / Duration, 0.f, 1.f);

	auto UpdateThrustHand = [&](FYoYoRuntime& Hand)
	{
		if (!Hand.bActive || !Hand.Component.IsValid())
		{
			return;
		}

		const FVector End = bYoYoReturning ? GetRestWorldLocation(Hand) : Hand.OutboundWorld;
		const FVector Start = bYoYoReturning ? Hand.OutboundWorld : Hand.PathStartWorld;
		Hand.Component->SetWorldLocation(FMath::Lerp(Start, End, Alpha));
	};

	UpdateThrustHand(RightHand);
	UpdateThrustHand(LeftHand);

	if (Alpha < 1.f)
	{
		return;
	}

	if (!bYoYoReturning)
	{
		StartYoYoReturn();
		ThrustElapsed = 0.f;
		return;
	}

	FinishYoYoPresentation();
}

void AEriCharacter::FinishYoYoPresentation()
{
	RightHand.bActive = false;
	LeftHand.bActive = false;

	StopYoYoAttackVFX();

	PresentationMode = EYoYoPresentationMode::None;
	bYoYoReturning = false;
	OrbitTravelDegrees = 0.f;
	ThrustElapsed = 0.f;
	ThrustDuration = 0.f;

	AttachYoYosToHandSockets();

	// Next buffered attack may start now.
	if (AttackComponent)
	{
		AttackComponent->NotifyPresentationComplete();
	}

	if (PresentationMode == EYoYoPresentationMode::None)
	{
		ClearPresentationAttackType();
	}
}

EAttackType AEriCharacter::GetActiveAttackType() const
{
	return PresentationAttackType;
}

void AEriCharacter::ClearPresentationAttackType()
{
	PresentationAttackType = EAttackType::None;
}

void AEriCharacter::StartYoYoAttackVFX()
{
	if (!CurrentYoYoAttackVFX)
	{
		return;
	}

	if (RightHand.bActive && YoYoRightVFX)
	{
		YoYoRightVFX->SetAsset(CurrentYoYoAttackVFX);
		YoYoRightVFX->Activate(true);
	}

	if (LeftHand.bActive && YoYoLeftVFX)
	{
		YoYoLeftVFX->SetAsset(CurrentYoYoAttackVFX);
		YoYoLeftVFX->Activate(true);
	}
}

void AEriCharacter::StopYoYoAttackVFX()
{
	if (YoYoRightVFX)
	{
		YoYoRightVFX->Deactivate();
	}

	if (YoYoLeftVFX)
	{
		YoYoLeftVFX->Deactivate();
	}
}

void AEriCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	DoMove(MovementVector.X, MovementVector.Y);
}

void AEriCharacter::JumpPressed()
{
	Jump();
}

void AEriCharacter::JumpReleased()
{
	StopJumping();
}

void AEriCharacter::Look(const FInputActionValue& Value)
{
	// Homing only has a back-facing sprite — lock view behind Eri during the dash.
	if (bHomingCameraLocked)
	{
		if (!bHomingCameraLockPitch)
		{
			const FVector2D LookVector = Value.Get<FVector2D>();
			AddControllerPitchInput(LookVector.Y);
		}
		return;
	}

	const FVector2D LookVector = Value.Get<FVector2D>();
	AddControllerPitchInput(LookVector.Y);
	AddControllerYawInput(LookVector.X);
}

void AEriCharacter::TryAttack()
{
	// Air + soft-lock target → homing dash (original flow).
	// Ground, or air without target → light combo.
	if (HomingAttackComponent
		&& HomingAttackComponent->GetHomingState() == EHomingState::TargetFound
		&& CharacterStateComponent
		&& CharacterStateComponent->GetLocomotionState() == ECharacterLocomotionState::Airborne)
	{
		TryHomingAttack();
		return;
	}

	DoAttack(EAttackType::Normal);
}

void AEriCharacter::TryAreaAttack()
{
	// Heavy / area works grounded and airborne (never hijacked by homing).
	DoAttack(EAttackType::Area);
}

void AEriCharacter::TryHomingAttack()
{
	if (!HomingAttackComponent || HomingAttackComponent->GetHomingState() != EHomingState::TargetFound)
	{
		return;
	}

	if (CharacterStateComponent
		&& CharacterStateComponent->GetLocomotionState() != ECharacterLocomotionState::Airborne)
	{
		return;
	}

	if (CharacterStateComponent)
	{
		CharacterStateComponent->SetActionState(ECharacterActionState::Homing);
	}

	SetHomingCameraLocked(true);
	HomingAttackComponent->DoHomingAttack();
	BeginHomingYoyoCharge();
}

void AEriCharacter::EnterTrickMode()
{
	if (CharacterStateComponent)
	{
		CharacterStateComponent->SetActionState(ECharacterActionState::Trick);
	}
}

void AEriCharacter::ExitTrickMode()
{
	if (CharacterStateComponent)
	{
		CharacterStateComponent->SetActionState(ECharacterActionState::Default);
	}
}

void AEriCharacter::TryTrickInput(const FInputActionValue& Value)
{
	if (CharacterStateComponent && CharacterStateComponent->GetActionState() == ECharacterActionState::Trick)
	{
		FVector2D TrickInputVector = Value.Get<FVector2D>();
		(void)TrickInputVector;
	}
}

void AEriCharacter::HandleHomingAttackFinished(const bool bSuccess)
{
	// Bounce or cancel: free camera (falling / grounded sprites are free to orbit).
	SetHomingCameraLocked(false);

	if (CharacterStateComponent)
	{
		CharacterStateComponent->SetLocomotionState(
			bSuccess ? ECharacterLocomotionState::Airborne : ECharacterLocomotionState::Grounded);
		CharacterStateComponent->SetActionState(ECharacterActionState::Default);
	}

	if (PresentationMode == EYoYoPresentationMode::Homing)
	{
		StartYoYoReturn();
	}
}

void AEriCharacter::SetHomingCameraLocked(bool bLocked)
{
	// No snap — Tick lerps yaw toward flight direction so the lock is readable.
	bHomingCameraLocked = bLocked;
}

FVector AEriCharacter::GetHomingCameraFacingDirection() const
{
	if (HomingAttackComponent)
	{
		if (AActor* Target = HomingAttackComponent->GetCurrentHomingTarget())
		{
			FVector TargetLocation = Target->GetActorLocation();
			if (Target->Implements<UHomingable>())
			{
				TargetLocation = IHomingable::Execute_GetTargetLocation(Target);
			}

			const FVector ToTarget = (TargetLocation - GetActorLocation()).GetSafeNormal2D();
			if (!ToTarget.IsNearlyZero())
			{
				return ToTarget;
			}
		}
	}

	FVector VelocityDir = GetVelocity().GetSafeNormal2D();
	if (!VelocityDir.IsNearlyZero())
	{
		return VelocityDir;
	}

	return GetActorForwardVector().GetSafeNormal2D();
}

void AEriCharacter::UpdateHomingCameraLock(float DeltaTime)
{
	if (!bHomingCameraLocked || DeltaTime <= 0.f)
	{
		return;
	}

	AController* CharController = GetController();
	if (!CharController)
	{
		return;
	}

	const FVector Facing = GetHomingCameraFacingDirection();
	if (Facing.IsNearlyZero())
	{
		return;
	}

	const FRotator DesiredRot = Facing.Rotation();
	FRotator ControlRot = CharController->GetControlRotation();

	if (HomingCameraYawInterpSpeed <= 0.f)
	{
		// 0 = explicit snap (debug / feel override).
		ControlRot.Yaw = DesiredRot.Yaw;
	}
	else
	{
		ControlRot.Yaw = FMath::FixedTurn(
			ControlRot.Yaw,
			DesiredRot.Yaw,
			HomingCameraYawInterpSpeed * DeltaTime
		);
	}

	CharController->SetControlRotation(ControlRot);
}

void AEriCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MovementAction)
		{
			EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &AEriCharacter::Move);
		}

		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AEriCharacter::Look);
		}

		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AEriCharacter::JumpPressed);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AEriCharacter::JumpReleased);
		}

		if (AttackAction)
		{
			EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AEriCharacter::TryAttack);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s has no AttackAction assigned"), *GetName());
		}

		if (AreaAttackAction)
		{
			EnhancedInputComponent->BindAction(AreaAttackAction, ETriggerEvent::Started, this, &AEriCharacter::TryAreaAttack);
		}

		// Optional dedicated homing button. Attack in air+target still homes if this is unset.
		if (HomingAction)
		{
			EnhancedInputComponent->BindAction(HomingAction, ETriggerEvent::Started, this, &AEriCharacter::TryHomingAttack);
		}

		if (TrickModeAction)
		{
			EnhancedInputComponent->BindAction(TrickModeAction, ETriggerEvent::Started, this, &AEriCharacter::EnterTrickMode);
			EnhancedInputComponent->BindAction(TrickModeAction, ETriggerEvent::Completed, this, &AEriCharacter::ExitTrickMode);
		}
		
		//debug
		if (TrickInputAction)
		{
			EnhancedInputComponent->BindAction(
			   TrickInputAction,
			   ETriggerEvent::Triggered,
			   this,
			   &AEriCharacter::TryTrickInput
			);
		}

		// Temporary Trick Aura VFX debug input.
		if (TrickAuraDebugAction)
		{
			EnhancedInputComponent->BindAction(
			   TrickAuraDebugAction,
			   ETriggerEvent::Started,
			   this,
			   &AEriCharacter::ToggleTrickAuraVFX
			);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s is not using an EnhancedInputComponent"), *GetName());
	}
}

void AEriCharacter::ToggleTrickAuraVFX()
{
	if (!TrickAuraVFX)
	{
		return;
	}

	if (!TrickAuraVFX->GetAsset())
	{
		return;
	}

	if (TrickAuraVFX->IsActive())
	{
		TrickAuraVFX->Deactivate();
	}
	else
	{
		TrickAuraVFX->Activate(true);
	}
}