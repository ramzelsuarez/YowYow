// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterBase.h"
#include "ActorComponents/AttackComponent.h"
#include "ActorComponents/HealthComponent.h"
#include "ActorComponents/SpriteDirectionComponent.h"
#include "CameraManagers/SpinningRiotCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "PaperFlipbookComponent.h"
#include "ActorComponents/CharacterStateComponent.h"
#include "Camera/CameraShakeBase.h"

ACharacterBase::ACharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
}

void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// Components are added manually in BPs — never spawn them from C++ (avoids duplicates).
	AttackComponent = FindComponentByClass<UAttackComponent>();
	HealthComponent = FindComponentByClass<UHealthComponent>();
	CharacterStateComponent = FindComponentByClass<UCharacterStateComponent>();
	SpriteDirectionComponent = FindComponentByClass<USpriteDirectionComponent>();

	if (!AttackComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s missing UAttackComponent — add it on the Blueprint."), *GetName());
	}
	if (!HealthComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s missing UHealthComponent — add it on the Blueprint."), *GetName());
	}
	if (!CharacterStateComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s missing UCharacterStateComponent — add it on the Blueprint."), *GetName());
	}

	if (HealthComponent)
	{
		HealthComponent->OnHealthDepleted.AddDynamic(this, &ACharacterBase::HandleHealthDepleted);
		HealthComponent->OnHealthDamageTaken.AddDynamic(this, &ACharacterBase::HandleHealthDamageTaken);
	}

	LandedDelegate.AddDynamic(this, &ACharacterBase::HandleLanded);
	MovementModeChangedDelegate.AddDynamic(this, &ACharacterBase::HandleMovementModeChanged);

	CameraManager = Cast<ASpinningRiotCameraManager>(UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0));

	if (CameraManager)
	{
		CameraManager->OnCameraRotationChanged.AddUObject(this, &ACharacterBase::HandleCameraRotationChanged);
		HandleCameraRotationChanged(CameraManager->GetCameraRotation());
	}
}

void ACharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	LandedDelegate.RemoveDynamic(this, &ACharacterBase::HandleLanded);
	MovementModeChangedDelegate.RemoveDynamic(this, &ACharacterBase::HandleMovementModeChanged);

	if (HealthComponent)
	{
		HealthComponent->OnHealthDepleted.RemoveDynamic(this, &ACharacterBase::HandleHealthDepleted);
		HealthComponent->OnHealthDamageTaken.RemoveDynamic(this, &ACharacterBase::HandleHealthDamageTaken);
	}

	if (CameraManager)
	{
		CameraManager->OnCameraRotationChanged.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

float ACharacterBase::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser
)
{
	if (HealthComponent && HealthComponent->IsDead())
	{
		return 0.f;
	}

	if (CharacterStateComponent && CharacterStateComponent->GetLifeState() == ECharacterLifeState::Dead)
	{
		return 0.f;
	}

	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void ACharacterBase::DoMove(float Right, float Forward)
{
	if (!CanMove()) return;

	if (GetController() != nullptr)
	{
		const FRotator ControlRotation = GetControlRotation();

		// yaw (horizontal) rotation, create a rotator that represents the controller's rotation
		// only using yaw because we don't want the character to lay down or else
		const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

		// given the yaw rotation, give me the X unit in the matrix (forward)
		// X is forward in UE!!
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// same thing, given the yaw rotation give me the Y, since Y is right in UE
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// scale that forward by mov vector Y because we have Y axis representing fw/bw directions
		AddMovementInput(ForwardDirection, Forward);
		// and scale by our input X, which represents X (horizontal) movement
		AddMovementInput(RightDirection, Right);
	}
}

void ACharacterBase::DoAttack(EAttackType AttackType)
{
	if (!AttackComponent)
	{
		return;
	}

	if (HealthComponent && HealthComponent->IsDead())
	{
		return;
	}

	if (CharacterStateComponent && CharacterStateComponent->GetLifeState() == ECharacterLifeState::Dead)
	{
		return;
	}

	// AttackComponent owns attack/action states, facing lock, and recovery.
	AttackComponent->TryAttack(AttackType);
}

void ACharacterBase::Jump()
{
	// jump is a default UE method for ACharacter
	// any custom jumping logic (i.e should the character be jumping at this point?) should go here before the super call
	// same for StopJumping down below
	Super::Jump();
}

void ACharacterBase::StopJumping()
{
	Super::StopJumping();
}

bool ACharacterBase::CanMove()
{
	if (!CharacterStateComponent)
	{
		return true;
	}

	// Block locomotion during active hit window and post-hit recovery (area anti-spam).
	const ECharacterAttackState AttackState = CharacterStateComponent->GetAttackState();
	return AttackState == ECharacterAttackState::None;
}

void ACharacterBase::HandleCameraRotationChanged(const FRotator& CameraRotation)
{
	if (UPaperFlipbookComponent* PaperFlipbookComponent = GetSprite())
	{
		// add +90 degrees because the sprite faces "right" by default
		PaperFlipbookComponent->SetWorldRotation(FRotator(0.f, CameraRotation.Yaw + 90.f, 0.f));
	}
}

void ACharacterBase::HandleMovementModeChanged(ACharacter* Character, EMovementMode PrevMovementMode,
                                               uint8 PreviousCustomMode)
{
	if (!CharacterStateComponent || Character != this)
	{
		return;
	}

	const EMovementMode CurrentMovementMode = GetCharacterMovement()->MovementMode;

	if (CurrentMovementMode == MOVE_Falling)
	{
		CharacterStateComponent->SetLocomotionState(ECharacterLocomotionState::Airborne);
	}
}

void ACharacterBase::HandleLanded(const FHitResult& Hit)
{
	if (CharacterStateComponent)
	{
		CharacterStateComponent->SetLocomotionState(ECharacterLocomotionState::Grounded);
	}
}

void ACharacterBase::HandleHealthDepleted(UHealthComponent* InHealthComponent, AActor* DamageCauser)
{
	if (CharacterStateComponent)
	{
		CharacterStateComponent->SetLifeState(ECharacterLifeState::Dead);
		CharacterStateComponent->SetAttackState(ECharacterAttackState::None);
		CharacterStateComponent->SetActionState(ECharacterActionState::Default);
	}
}

void ACharacterBase::HandleHealthDamageTaken(
	UHealthComponent* InHealthComponent,
	int32 Damage,
	int32 InCurrentHealth,
	AActor* DamageCauser,
	AController* DamageInstigator
)
{
	if (!DamageCameraShake || Damage <= 0)
	{
		return;
	}

	// Hurt feedback is player-only (3 HP game — getting hit is rare).
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}

	PlayerController->ClientStartCameraShake(DamageCameraShake, DamageCameraShakeScale);
}
