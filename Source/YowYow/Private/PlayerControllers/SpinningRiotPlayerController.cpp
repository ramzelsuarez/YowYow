// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllers/SpinningRiotPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"

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

void ASpinningRiotPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInput || !PauseAction)
	{
		return;
	}

	EnhancedInput->BindAction(PauseAction, ETriggerEvent::Started, this, &ASpinningRiotPlayerController::TogglePauseMenu);
}

void ASpinningRiotPlayerController::TogglePauseMenu()
{
	if (bPauseMenuOpen)
	{
		ClosePauseMenu();
	}
	else
	{
		OpenPauseMenu();
	}
}

void ASpinningRiotPlayerController::OpenPauseMenu()
{
	if (bPauseMenuOpen || !PauseMenuClass)
	{
		return;
	}

	PauseMenuWidget = CreateWidget<UUserWidget>(this, PauseMenuClass);
	if (!PauseMenuWidget)
	{
		return;
	}

	PauseMenuWidget->AddToViewport(100);
	bPauseMenuOpen = true;

	SetPause(true);
	bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(PauseMenuWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void ASpinningRiotPlayerController::ClosePauseMenu()
{
	if (PauseMenuWidget)
	{
		PauseMenuWidget->RemoveFromParent();
		PauseMenuWidget = nullptr;
	}

	bPauseMenuOpen = false;
	SetPause(false);
	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());
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
