// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/EriCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "ActorComponents/CharacterStateComponent.h"
#include "ActorComponents/HomingAttackComponent.h"

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
}

void AEriCharacter::BeginPlay()
{
	Super::BeginPlay();

	HomingAttackComponent = FindComponentByClass<UHomingAttackComponent>();

	if (HomingAttackComponent)
	{
		HomingAttackComponent->OnHomingAttackFinished.AddDynamic(this, &AEriCharacter::HandleHomingAttackFinished);
	}
}

void AEriCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
	const FVector2D LookVector = Value.Get<FVector2D>();

	AddControllerPitchInput(LookVector.Y);
	AddControllerYawInput(LookVector.X);
}

void AEriCharacter::TryAttack()
{
	// TODO: add any gating to attacking here
	// TODO: if (HomingAttackComponent->bHasHomingTarget) HomingAttackComponent->DoHomingAttack()
	if (HomingAttackComponent && HomingAttackComponent->GetHomingState() == EHomingState::TargetFound)
	{
		if (CharacterStateComponent)
		{
			CharacterStateComponent->SetActionState(ECharacterActionState::Homing);
		}

		HomingAttackComponent->DoHomingAttack();
	}
	else
	{
		DoAttack(EAttackType::Normal);
	}
}

void AEriCharacter::TryAreaAttack()
{
	// TODO: add any gating to area attacking here
	DoAttack(EAttackType::Area);
}

void AEriCharacter::EnterTrickMode()
{
	CharacterStateComponent->SetActionState(ECharacterActionState::Trick);

	// TODO: TrickGaugeComponent->DoEnterTrickMode()
}

void AEriCharacter::ExitTrickMode()
{
	CharacterStateComponent->SetActionState(ECharacterActionState::Default);

	// TODO: TrickGaugeComponent->DoExitTrickMode();
}

void AEriCharacter::TryTrickInput(const FInputActionValue& Value)
{
	if (CharacterStateComponent->GetActionState() == ECharacterActionState::Trick)
	{
		FVector2D TrickInputVector = Value.Get<FVector2D>();

		// TODO: TrickGaugeComponent->RegisterTrickInput(TrickInputVector);
	}
}

void AEriCharacter::HandleHomingAttackFinished(const bool bSuccess)
{
	// TODO: provide better calculation depending on what actually happened
	CharacterStateComponent->SetLocomotionState(bSuccess ? ECharacterLocomotionState::Airborne : ECharacterLocomotionState::Grounded);
	CharacterStateComponent->SetActionState(ECharacterActionState::Default);
}

void AEriCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// movement related actions
		if (MovementAction)
		{
			EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &AEriCharacter::Move);
		}

		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AEriCharacter::Look);
		}

		// override inherited jump actions
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AEriCharacter::JumpPressed);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this,
			                                   &AEriCharacter::JumpReleased);
		}

		// combat actions
		if (AttackAction)
		{
			EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AEriCharacter::TryAttack);
		}
		if (AreaAttackAction)
		{
			EnhancedInputComponent->BindAction(AreaAttackAction, ETriggerEvent::Triggered, this,
			                                   &AEriCharacter::TryAreaAttack);
		}

		// trick gauge action
		if (TrickModeAction)
		{
			EnhancedInputComponent->BindAction(TrickModeAction, ETriggerEvent::Started, this,
			                                   &AEriCharacter::EnterTrickMode);
			EnhancedInputComponent->BindAction(TrickModeAction, ETriggerEvent::Completed, this,
			                                   &AEriCharacter::ExitTrickMode);
		}
		if (TrickInputAction)
		{
			EnhancedInputComponent->BindAction(TrickInputAction, ETriggerEvent::Triggered, this,
			                                   &AEriCharacter::TryTrickInput);
		}
	}
}
