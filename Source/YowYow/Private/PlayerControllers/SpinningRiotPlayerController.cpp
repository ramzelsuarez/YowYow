// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllers/SpinningRiotPlayerController.h"

#include "ActorComponents/EnemyAIComponent.h"
#include "ActorComponents/HealthComponent.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "GameModes/SpinningRiot.h"
#include "Engine/World.h"
#include "TimerManager.h"

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

	Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem && TrickModeIMC) Subsystem->RemoveMappingContext(TrickModeIMC);
	if (Subsystem && GameplayIMC)
	{
		Subsystem->AddMappingContext(GameplayIMC, 0);
	}
	if (ASpinningRiot* Demo = GetWorld()->GetAuthGameMode<ASpinningRiot>())
	{
		Demo->RefreshDemoInput();
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
	if (!IsCombatPhase()) return;
	UEnemyAIComponent::ToggleGlobalAIFrozen();
}

void ASpinningRiotPlayerController::TogglePauseMenu()
{
	if (IsPossessedPawnDead() || !IsCombatPhase())
	{
		return;
	}

	if (bPauseMenuOpen)
	{
		ClosePauseMenu();
	}
	else
	{
		OpenPauseMenu();
	}
}

void ASpinningRiotPlayerController::OpenPauseMenuOnDeath()
{
	// Deprecated Blueprint entry point. Death UI is owned by the demo GameMode.
}

void ASpinningRiotPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(DeathPauseMenuTimerHandle);
	if (PauseMenuWidget) PauseMenuWidget->RemoveFromParent();
	if (Subsystem)
	{
		if (TrickModeIMC) Subsystem->RemoveMappingContext(TrickModeIMC);
		if (GameplayIMC) Subsystem->RemoveMappingContext(GameplayIMC);
	}
	Super::EndPlay(EndPlayReason);
}

void ASpinningRiotPlayerController::OpenPauseMenu()
{
	GetWorldTimerManager().ClearTimer(DeathPauseMenuTimerHandle);

	if (bPauseMenuOpen || !PauseMenuClass || IsPossessedPawnDead() || !IsCombatPhase())
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
	if (IsPossessedPawnDead() || !IsCombatPhase())
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
	if (CanUseTrickInputContext())
	{
		FModifyContextOptions Options;
		Options.bIgnoreAllPressedKeysUntilRelease = false;
		Subsystem->RemoveMappingContext(GameplayIMC, Options);
		Subsystem->AddMappingContext(TrickModeIMC, 10, Options);
	}
}

void ASpinningRiotPlayerController::ExitTrickMode() const
{
	if (Subsystem && TrickModeIMC)
	{
		Subsystem->RemoveMappingContext(TrickModeIMC);
		if (GameplayIMC && IsCombatPhase()) Subsystem->AddMappingContext(GameplayIMC, 0);
	}
}

bool ASpinningRiotPlayerController::CanUseTrickInputContext() const
{
	return Subsystem && GameplayIMC && TrickModeIMC && GameplayIMC != TrickModeIMC;
}

bool ASpinningRiotPlayerController::IsCombatPhase() const
{
	const ASpinningRiot* Demo = GetWorld() ? GetWorld()->GetAuthGameMode<ASpinningRiot>() : nullptr;
	return !Demo || Demo->GetDemoPhase() == EDemoPhase::Combat;
}

void ASpinningRiotPlayerController::ApplyDemoInputMode(bool bCombat, UUserWidget* PhaseWidget)
{
	if (!IsLocalPlayerController()) return;
	GetWorldTimerManager().ClearTimer(DeathPauseMenuTimerHandle);
	if (PauseMenuWidget)
	{
		PauseMenuWidget->RemoveFromParent();
		PauseMenuWidget = nullptr;
	}
	if (bPauseMenuOpen) SetPause(false);
	bPauseMenuOpen = false;
	ResetIgnoreMoveInput();
	ResetIgnoreLookInput();
	SetIgnoreMoveInput(!bCombat);
	SetIgnoreLookInput(!bCombat);
	bShowMouseCursor = !bCombat;
	if (bCombat)
	{
		SetInputMode(FInputModeGameOnly());
	}
	else
	{
		FInputModeUIOnly InputMode;
		if (PhaseWidget) InputMode.SetWidgetToFocus(PhaseWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
}
