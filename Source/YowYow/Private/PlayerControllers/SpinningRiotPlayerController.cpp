// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllers/SpinningRiotPlayerController.h"
#include "EnhancedInputSubsystems.h"

void ASpinningRiotPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalPlayerController() || !GameplayIMC) return;

	Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

	if (Subsystem)
	{
		Subsystem->AddMappingContext(GameplayIMC, 0);
	}
}

void ASpinningRiotPlayerController::EnterTrickMode() const
{
	if (Subsystem && TrickModeIMC)
	{
		Subsystem->AddMappingContext(TrickModeIMC, 10);
	}
}

void ASpinningRiotPlayerController::ExitTrickMode() const
{
	if (Subsystem && TrickModeIMC)
	{
		Subsystem->RemoveMappingContext(TrickModeIMC);
	}
}
