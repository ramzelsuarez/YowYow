#include "GameModes/SpinningRiot.h"

#include "ActorComponents/AttackComponent.h"
#include "ActorComponents/ComboComponent.h"
#include "ActorComponents/EnemyAIComponent.h"
#include "BattleSystem/WaveEnemyManager.h"
#include "Blueprint/UserWidget.h"
#include "Characters/EriCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerControllers/SpinningRiotPlayerController.h"
#include "TimerManager.h"
#include "Engine/World.h"

void ASpinningRiot::BeginPlay()
{
	Super::BeginPlay();
	WaveManager = Cast<AWaveEnemyManager>(UGameplayStatics::GetActorOfClass(this, AWaveEnemyManager::StaticClass()));
	if (WaveManager)
	{
		WaveManager->OnEncounterCompleted.AddDynamic(this, &ASpinningRiot::HandleEncounterCompleted);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s found no WaveEnemyManager."), *GetName());
	}
	SetDemoPhase(EDemoPhase::Play);
	if (WaveManager && WaveManager->IsEncounterCompleted()) HandleEncounterCompleted();
}

void ASpinningRiot::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(DemoWinTimer);
	if (WaveManager) WaveManager->OnEncounterCompleted.RemoveDynamic(this, &ASpinningRiot::HandleEncounterCompleted);
	if (ActivePhaseWidget) ActivePhaseWidget->RemoveFromParent();
	UEnemyAIComponent::SetGlobalAIFrozen(false);
	Super::EndPlay(EndPlayReason);
}

void ASpinningRiot::RefreshDemoInput()
{
	ASpinningRiotPlayerController* DemoController = Cast<ASpinningRiotPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
	if (!DemoController) return;
	if (!ActivePhaseWidget)
	{
		TSubclassOf<UUserWidget> PhaseClass;
		if (DemoPhase == EDemoPhase::Win) PhaseClass = GameWonWidgetClass;
		else if (DemoPhase == EDemoPhase::Lose) PhaseClass = GameOverWidgetClass;
		if (PhaseClass)
		{
			ActivePhaseWidget = CreateWidget<UUserWidget>(DemoController, PhaseClass);
			if (ActivePhaseWidget) ActivePhaseWidget->AddToViewport(200);
		}
	}
	DemoController->ApplyDemoInputMode(DemoPhase == EDemoPhase::Play, ActivePhaseWidget);
}

void ASpinningRiot::SetDemoPhase(EDemoPhase NewPhase)
{
	if (bDemoPhaseInitialized && NewPhase == DemoPhase) return;
	const EDemoPhase OldPhase = DemoPhase;
	DemoPhase = NewPhase;
	bDemoPhaseInitialized = true;
	if (ActivePhaseWidget)
	{
		ActivePhaseWidget->RemoveFromParent();
		ActivePhaseWidget = nullptr;
	}
	const bool bCombat = DemoPhase == EDemoPhase::Play;
	UEnemyAIComponent::SetGlobalAIFrozen(!bCombat);
	if (!bCombat)
	{
		TArray<AActor*> Characters;
		UGameplayStatics::GetAllActorsOfClass(this, ACharacterBase::StaticClass(), Characters);
		for (AActor* Actor : Characters)
		{
			ACharacterBase* Character = Cast<ACharacterBase>(Actor);
			if (AEriCharacter* Eri = Cast<AEriCharacter>(Character)) Eri->CancelDemoActions();
			else if (UAttackComponent* Attack = Character->FindComponentByClass<UAttackComponent>()) Attack->CancelActiveAttack();
			Character->GetCharacterMovement()->StopMovementImmediately();
			Character->ConsumeMovementInputVector();
		}
	}
	RefreshDemoInput();
	OnDemoPhaseChanged.Broadcast(OldPhase, NewPhase);
}

void ASpinningRiot::HandlePlayerDied()
{
	if (DemoPhase != EDemoPhase::Play) return;
	GetWorldTimerManager().ClearTimer(DemoWinTimer);
	SetDemoPhase(EDemoPhase::Lose);
}

void ASpinningRiot::HandleEncounterCompleted()
{
	if (DemoPhase != EDemoPhase::Play || GetWorldTimerManager().TimerExists(DemoWinTimer)) return;
	// Leave the damage callback before canceling presentations and creating result UI.
	DemoWinTimer = GetWorldTimerManager().SetTimerForNextTick(this, &ASpinningRiot::CompleteDemoWin);
}

void ASpinningRiot::CompleteDemoWin()
{
	if (DemoPhase != EDemoPhase::Play) return;
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (const ACharacterBase* PlayerCharacter = Cast<ACharacterBase>(PlayerPawn))
	{
		if (PlayerCharacter->IsDead())
		{
			HandlePlayerDied();
			return;
		}
	}
	if (PlayerPawn)
	{
		if (UComboComponent* Combo = PlayerPawn->FindComponentByClass<UComboComponent>())
		{
			WonComboTier = Combo->GetCurrentTier();
			WonComboHits = Combo->GetHitCount();
			WonComboPoints = Combo->GetCurrentPoints();
		}
	}
	SetDemoPhase(EDemoPhase::Win);
}
