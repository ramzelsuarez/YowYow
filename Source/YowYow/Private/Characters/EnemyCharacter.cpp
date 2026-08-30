// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/EnemyCharacter.h"

#include "ActorComponents/CharacterStateComponent.h"
#include "ActorComponents/HealthComponent.h"
#include "BattleSystem/WaveEnemyManager.h"
#include "CharacterStates/CharacterStates.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComponent)
	{
		HealthComponent->OnHealthDepleted.AddDynamic(this, &AEnemyCharacter::HandleEnemyHealthDepleted);
	}

	if (!WaveManager)
	{
		WaveManager = Cast<AWaveEnemyManager>(
			UGameplayStatics::GetActorOfClass(GetWorld(), AWaveEnemyManager::StaticClass())
		);
	}
}

void AEnemyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CorpseDestroyTimerHandle);
	}

	if (HealthComponent)
	{
		HealthComponent->OnHealthDepleted.RemoveDynamic(this, &AEnemyCharacter::HandleEnemyHealthDepleted);
	}

	Super::EndPlay(EndPlayReason);
}

bool AEnemyCharacter::GetIsHomingTargeted_Implementation()
{
	return bIsHomingTargeted;
}

void AEnemyCharacter::SetHomingTargeted_Implementation(bool bTargeted)
{
	bIsHomingTargeted = bTargeted;
}

bool AEnemyCharacter::CanBeHomed_Implementation() const
{
	if (HealthComponent && HealthComponent->IsDead())
	{
		return false;
	}

	if (CharacterStateComponent && CharacterStateComponent->GetLifeState() == ECharacterLifeState::Dead)
	{
		return false;
	}

	return true;
}

FVector AEnemyCharacter::GetTargetLocation_Implementation()
{
	return GetActorLocation();
}

void AEnemyCharacter::HandleEnemyHealthDepleted(UHealthComponent* InHealthComponent, AActor* DamageCauser)
{
	if (WaveManager)
	{
		WaveManager->RegisterEnemyDefeated(this);
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		Destroy();
		return;
	}

	if (CorpseLifetime <= 0.f)
	{
		Destroy();
		return;
	}

	World->GetTimerManager().SetTimer(
		CorpseDestroyTimerHandle,
		this,
		&AEnemyCharacter::DestroyCorpse,
		CorpseLifetime,
		false
	);
}

void AEnemyCharacter::DestroyCorpse()
{
	Destroy();
}
