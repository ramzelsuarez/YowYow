// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/SpinningRiot.h"

#include "ActorComponents/EnemyAIComponent.h"
#include "BattleSystem/WaveEnemyManager.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/Class.h"

void ASpinningRiot::BeginPlay()
{
	Super::BeginPlay();

	// Static flag survives PIE stop/start; always start a match with AI on.
	UEnemyAIComponent::SetGlobalAIFrozen(false);

	WaveManager = Cast<AWaveEnemyManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AWaveEnemyManager::StaticClass())
	);

	if (!WaveManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s found no WaveEnemyManager — demo phase stays Combat."), *GetName());
		return;
	}

	WaveManager->OnEncounterCompleted.AddDynamic(this, &ASpinningRiot::HandleEncounterCompleted);

	if (WaveManager->IsEncounterCompleted())
	{
		HandleEncounterCompleted();
	}
}

void ASpinningRiot::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (WaveManager)
	{
		WaveManager->OnEncounterCompleted.RemoveDynamic(this, &ASpinningRiot::HandleEncounterCompleted);
	}

	Super::EndPlay(EndPlayReason);
}

void ASpinningRiot::SetDemoPhase(EDemoPhase NewPhase)
{
	if (NewPhase == DemoPhase)
	{
		return;
	}

	const EDemoPhase OldPhase = DemoPhase;
	DemoPhase = NewPhase;
	UE_LOG(LogTemp, Warning, TEXT("Demo phase: %s -> %s"),
		*UEnum::GetValueAsString(OldPhase),
		*UEnum::GetValueAsString(NewPhase)
	);
	OnDemoPhaseChanged.Broadcast(OldPhase, NewPhase);
}

void ASpinningRiot::HandleEncounterCompleted()
{
	SetDemoPhase(EDemoPhase::Puzzle);
}

