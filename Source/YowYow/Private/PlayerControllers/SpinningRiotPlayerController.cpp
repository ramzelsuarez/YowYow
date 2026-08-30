// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllers/SpinningRiotPlayerController.h"

#include "ActorComponents/EnemyAIComponent.h"
#include "ActorComponents/HealthComponent.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"

void ASpinningRiotPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalPlayerController())
	{
		return;
	}

	// Title_test leaves UIOnly + cursor on the viewport; OpenLevel does not clear it.
	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());

	if (!GameplayIMC)
	{
		return;
	}

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
	if (!EnhancedInput)
	{
		return;
	}

	if (PauseAction)
	{
		EnhancedInput->BindAction(PauseAction, ETriggerEvent::Started, this, &ASpinningRiotPlayerController::TogglePauseMenu);
	}

	if (ToggleEnemyAIAction)
	{
		EnhancedInput->BindAction(ToggleEnemyAIAction, ETriggerEvent::Started, this, &ASpinningRiotPlayerController::ToggleEnemyAI);
	}
}

void ASpinningRiotPlayerController::ToggleEnemyAI()
{
	UEnemyAIComponent::ToggleGlobalAIFrozen();
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
	if (IsPossessedPawnDead())
	{
		return;
	}

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

bool ASpinningRiotPlayerController::IsPossessedPawnDead() const
{
	const APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return false;
	}

	const UHealthComponent* Health = ControlledPawn->FindComponentByClass<UHealthComponent>();
	return Health && Health->IsDead();
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
