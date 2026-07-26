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

AEriCharacter::AEriCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.0f;
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
}

void AEriCharacter::BeginPlay()
{
	Super::BeginPlay();

	HomingAttackComponent = FindComponentByClass<UHomingAttackComponent>();
	TrickGaugeComponent = FindComponentByClass<UTrickGaugeComponent>();

	ApplyYoYoMeshAssets();
	CacheYoYoRests();

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

	Super::EndPlay(EndPlayReason);
}

void AEriCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
	RightHand.Component = YoYoRight;
	LeftHand.Component = YoYoLeft;

	if (YoYoRight)
	{
		RightHand.RestRelative = YoYoRight->GetRelativeLocation();
	}
	if (YoYoLeft)
	{
		LeftHand.RestRelative = YoYoLeft->GetRelativeLocation();
	}
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
	return GetActorTransform().TransformPosition(Hand.RestRelative);
}

void AEriCharacter::HandleAttackStarted(EAttackType AttackType, FAttackData StartedAttackData)
{
	BeginYoYoPresentation(StartedAttackData);
}

void AEriCharacter::HandleAttackFinished(EAttackType AttackType, bool bCompleted)
{
	// Hit window closed: begin return. Never snap home here.
	if (PresentationMode == EYoYoPresentationMode::None)
	{
		if (AttackComponent)
		{
			AttackComponent->NotifyPresentationComplete();
		}
		return;
	}

	StartYoYoReturn();
}

void AEriCharacter::BeginYoYoPresentation(const FAttackData& InAttackData)
{
	// Soft reset flags without snapping (rest already cached from BP).
	bYoYoReturning = false;
	OrbitAngleDegrees = 0.f;
	RightHand.bActive = false;
	LeftHand.bActive = false;

	// Re-read rest in case BP moved them (editor-only usually); safe each attack start.
	CacheYoYoRests();

	CachedAttackForward = GetActorForwardVector().GetSafeNormal2D();
	if (CachedAttackForward.IsNearlyZero())
	{
		CachedAttackForward = FVector::ForwardVector;
	}

	YoYoCurrentSpeed = InAttackData.Speed > 0.f ? InAttackData.Speed : 600.f;
	OrbitRadius = FMath::Max(InAttackData.Range, 1.f);

	TArray<FYoYoRuntime*> Hands;
	const EYoYoHand Hand = (InAttackData.Motion == EAttackMotion::OrbitCircle)
		? EYoYoHand::Both
		: InAttackData.YoYoHand;
	GatherHands(Hand, Hands);

	if (Hands.IsEmpty())
	{
		PresentationMode = EYoYoPresentationMode::None;
		if (AttackComponent)
		{
			AttackComponent->NotifyPresentationComplete();
		}
		return;
	}

	if (InAttackData.Motion == EAttackMotion::OrbitCircle)
	{
		PresentationMode = EYoYoPresentationMode::Orbit;
		float Phase = 0.f;
		for (FYoYoRuntime* HandRuntime : Hands)
		{
			HandRuntime->bActive = true;
			HandRuntime->OrbitPhaseOffsetDegrees = Phase;
			Phase += 180.f;
		}
		return;
	}

	// Thrust: go out along cached forward, then return home.
	PresentationMode = EYoYoPresentationMode::Thrust;
	const float Range = FMath::Max(InAttackData.Range, 1.f);
	for (FYoYoRuntime* HandRuntime : Hands)
	{
		HandRuntime->bActive = true;
		USceneComponent* Comp = HandRuntime->Component.Get();
		if (!Comp)
		{
			continue;
		}
		const FVector RestWorld = GetRestWorldLocation(*HandRuntime);
		Comp->SetWorldLocation(RestWorld);
		HandRuntime->OutboundWorld = RestWorld + CachedAttackForward * Range;
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

	if (PresentationMode == EYoYoPresentationMode::Orbit)
	{
		if (bYoYoReturning)
		{
			// Lerp both hands back to rest after orbit hit window ends.
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

		const float AngularSpeed = FMath::RadiansToDegrees(YoYoCurrentSpeed / FMath::Max(OrbitRadius, 1.f));
		OrbitAngleDegrees += AngularSpeed * DeltaTime;

		const FVector Center = GetActorLocation() + FVector::UpVector * 50.f;
		const FVector Forward = CachedAttackForward;
		const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();

		auto UpdateOrbitHand = [&](FYoYoRuntime& Hand)
		{
			if (!Hand.bActive || !Hand.Component.IsValid())
			{
				return;
			}
			const float AngleRad = FMath::DegreesToRadians(OrbitAngleDegrees + Hand.OrbitPhaseOffsetDegrees);
			const FVector Dir = Forward * FMath::Cos(AngleRad) + Right * FMath::Sin(AngleRad);
			Hand.Component->SetWorldLocation(Center + Dir * OrbitRadius);
		};

		UpdateOrbitHand(RightHand);
		UpdateOrbitHand(LeftHand);
		return;
	}

	// Thrust outbound / return
	bool bAnyStillMoving = false;

	auto UpdateThrustHand = [&](FYoYoRuntime& Hand)
	{
		if (!Hand.bActive || !Hand.Component.IsValid())
		{
			return;
		}

		USceneComponent* Comp = Hand.Component.Get();
		const FVector Target = bYoYoReturning ? GetRestWorldLocation(Hand) : Hand.OutboundWorld;
		const float Speed = bYoYoReturning ? YoYoCurrentSpeed * YoYoReturnSpeedMultiplier : YoYoCurrentSpeed;
		const FVector NewLocation = FMath::VInterpConstantTo(Comp->GetComponentLocation(), Target, DeltaTime, Speed);
		Comp->SetWorldLocation(NewLocation);

		if (!NewLocation.Equals(Target, 1.5f))
		{
			bAnyStillMoving = true;
		}
	};

	UpdateThrustHand(RightHand);
	UpdateThrustHand(LeftHand);

	if (bAnyStillMoving)
	{
		return;
	}

	if (!bYoYoReturning)
	{
		// Reached apex — auto start return even if hitbox hasn't closed yet.
		bYoYoReturning = true;
		return;
	}

	// Fully home.
	FinishYoYoPresentation();
}

void AEriCharacter::FinishYoYoPresentation()
{
	auto ResetHand = [](FYoYoRuntime& Hand)
	{
		if (Hand.Component.IsValid())
		{
			Hand.Component->SetRelativeLocation(Hand.RestRelative);
		}
		Hand.bActive = false;
	};

	ResetHand(RightHand);
	ResetHand(LeftHand);
	PresentationMode = EYoYoPresentationMode::None;
	bYoYoReturning = false;
	OrbitAngleDegrees = 0.f;

	// Next buffered attack may start now.
	if (AttackComponent)
	{
		AttackComponent->NotifyPresentationComplete();
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
	if (HomingAttackComponent && HomingAttackComponent->GetHomingState() == EHomingState::TargetFound)
	{
		if (CharacterStateComponent)
		{
			CharacterStateComponent->SetActionState(ECharacterActionState::Homing);
		}

		SetHomingCameraLocked(true);
		HomingAttackComponent->DoHomingAttack();
	}
	else
	{
		DoAttack(EAttackType::Normal);
	}
}

void AEriCharacter::TryAreaAttack()
{
	DoAttack(EAttackType::Area);
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
}

void AEriCharacter::SetHomingCameraLocked(bool bLocked)
{
	bHomingCameraLocked = bLocked;
	if (bLocked)
	{
		// Snap once so the first frame of dash already shows the back.
		UpdateHomingCameraLock(0.f);
	}
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
	if (!bHomingCameraLocked)
	{
		return;
	}

	AController* Controller = GetController();
	if (!Controller)
	{
		return;
	}

	const FVector Facing = GetHomingCameraFacingDirection();
	if (Facing.IsNearlyZero())
	{
		return;
	}

	const FRotator DesiredRot = Facing.Rotation();
	FRotator ControlRot = Controller->GetControlRotation();

	if (HomingCameraYawInterpSpeed <= 0.f || DeltaTime <= 0.f)
	{
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

	Controller->SetControlRotation(ControlRot);
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

		if (TrickModeAction)
		{
			EnhancedInputComponent->BindAction(TrickModeAction, ETriggerEvent::Started, this, &AEriCharacter::EnterTrickMode);
			EnhancedInputComponent->BindAction(TrickModeAction, ETriggerEvent::Completed, this, &AEriCharacter::ExitTrickMode);
		}
		if (TrickInputAction)
		{
			EnhancedInputComponent->BindAction(TrickInputAction, ETriggerEvent::Triggered, this, &AEriCharacter::TryTrickInput);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s is not using an EnhancedInputComponent"), *GetName());
	}
}
