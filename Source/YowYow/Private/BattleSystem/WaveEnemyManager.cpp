#include "BattleSystem/WaveEnemyManager.h"

#include "BattleSystem/EnemySpawnPoint.h"
#include "Characters/EnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AWaveEnemyManager::AWaveEnemyManager()
{
	PrimaryActorTick.bCanEverTick = false;

	// Demo defaults: 2, then 3, then 4. Assign DefaultEnemyClass (or per-entry class) in the editor.
	FEnemyWaveEntry AutoEntry;
	AutoEntry.SpawnPointIndex = -1;

	FEnemyWave Wave1;
	Wave1.DelayBeforeWave = 0.f;
	Wave1.Entries.Init(AutoEntry, 2);

	FEnemyWave Wave2;
	Wave2.DelayBeforeWave = 1.5f;
	Wave2.Entries.Init(AutoEntry, 3);

	FEnemyWave Wave3;
	Wave3.DelayBeforeWave = 1.5f;
	Wave3.Entries.Init(AutoEntry, 4);

	Waves = { Wave1, Wave2, Wave3 };
}

void AWaveEnemyManager::BeginPlay()
{
	Super::BeginPlay();

	if (!bAutoStart)
	{
		return;
	}

	if (EncounterStartDelay <= 0.f)
	{
		StartEncounter();
		return;
	}

	GetWorldTimerManager().SetTimer(
		EncounterStartTimerHandle,
		this,
		&AWaveEnemyManager::StartEncounter,
		EncounterStartDelay,
		false
	);
}

void AWaveEnemyManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(EncounterStartTimerHandle);
		World->GetTimerManager().ClearTimer(SpawnWaveTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

bool AWaveEnemyManager::RequestAttackToken(AActor* RequestingEnemy)
{
	if (!RequestingEnemy)
	{
		return false;
	}

	if (EnemiesWithAttackTokens.Contains(RequestingEnemy))
	{
		return true;
	}

	if (EnemiesWithAttackTokens.Num() >= MaxAttackTokens)
	{
		return false;
	}

	EnemiesWithAttackTokens.Add(RequestingEnemy);
	return true;
}

void AWaveEnemyManager::ReleaseAttackToken(AActor* ReleasingEnemy)
{
	if (!ReleasingEnemy)
	{
		return;
	}

	EnemiesWithAttackTokens.Remove(ReleasingEnemy);
}

void AWaveEnemyManager::RegisterEnemyDefeated(AActor* DefeatedEnemy)
{
	ReleaseAttackToken(DefeatedEnemy);

	DefeatedEnemies++;

	const int32 Removed = AliveEnemies.RemoveSingle(DefeatedEnemy);
	UE_LOG(LogTemp, Warning, TEXT("Enemy defeated: %d (alive in wave: %d)"), DefeatedEnemies, AliveEnemies.Num());

	if (Removed <= 0 || AliveEnemies.Num() > 0 || bEncounterCompleted)
	{
		return;
	}

	AdvanceWave();
}

void AWaveEnemyManager::StartEncounter()
{
	if (bEncounterStarted || bEncounterCompleted)
	{
		return;
	}

	bEncounterStarted = true;
	CurrentWaveIndex = -1;

	if (Waves.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no waves — completing encounter."), *GetName());
		CompleteEncounter();
		return;
	}

	AdvanceWave();
}

void AWaveEnemyManager::AdvanceWave()
{
	if (bEncounterCompleted)
	{
		return;
	}

	CurrentWaveIndex++;
	if (!Waves.IsValidIndex(CurrentWaveIndex))
	{
		CompleteEncounter();
		return;
	}

	const float Delay = Waves[CurrentWaveIndex].DelayBeforeWave;
	if (Delay <= 0.f)
	{
		SpawnCurrentWave();
		return;
	}

	GetWorldTimerManager().SetTimer(
		SpawnWaveTimerHandle,
		this,
		&AWaveEnemyManager::SpawnCurrentWave,
		Delay,
		false
	);
}

void AWaveEnemyManager::SpawnCurrentWave()
{
	if (bEncounterCompleted || !Waves.IsValidIndex(CurrentWaveIndex))
	{
		return;
	}

	const FEnemyWave& Wave = Waves[CurrentWaveIndex];
	TSet<int32> UsedThisWave;

	int32 SpawnedCount = 0;
	for (const FEnemyWaveEntry& Entry : Wave.Entries)
	{
		const TSubclassOf<AEnemyCharacter> EnemyClass = ResolveEnemyClass(Entry);
		if (!EnemyClass)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("%s wave %d skipped an entry — no EnemyClass and no DefaultEnemyClass."),
				*GetName(),
				CurrentWaveIndex
			);
			continue;
		}

		AEnemySpawnPoint* SpawnPoint = PickSpawnPoint(Entry.SpawnPointIndex, UsedThisWave);
		SpawnEnemy(EnemyClass, SpawnPoint);
		++SpawnedCount;
	}

	UE_LOG(LogTemp, Warning, TEXT("%s spawned wave %d (%d actors)"), *GetName(), CurrentWaveIndex, SpawnedCount);

	if (AliveEnemies.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s wave %d spawned nobody — advancing."), *GetName(), CurrentWaveIndex);
		AdvanceWave();
	}
}

void AWaveEnemyManager::CompleteEncounter()
{
	if (bEncounterCompleted)
	{
		return;
	}

	bEncounterCompleted = true;
	GetWorldTimerManager().ClearTimer(SpawnWaveTimerHandle);
	GetWorldTimerManager().ClearTimer(EncounterStartTimerHandle);

	UE_LOG(LogTemp, Warning, TEXT("Encounter complete."));
	OnEncounterCompleted.Broadcast();
}

TSubclassOf<AEnemyCharacter> AWaveEnemyManager::ResolveEnemyClass(const FEnemyWaveEntry& Entry) const
{
	return Entry.EnemyClass ? Entry.EnemyClass : DefaultEnemyClass;
}

AEnemySpawnPoint* AWaveEnemyManager::PickSpawnPoint(int32 RequestedIndex, TSet<int32>& UsedThisWave) const
{
	if (SpawnPoints.IsValidIndex(RequestedIndex) && SpawnPoints[RequestedIndex])
	{
		UsedThisWave.Add(RequestedIndex);
		return SpawnPoints[RequestedIndex];
	}

	TArray<int32> Candidates;
	Candidates.Reserve(SpawnPoints.Num());
	for (int32 Index = 0; Index < SpawnPoints.Num(); ++Index)
	{
		if (SpawnPoints[Index])
		{
			Candidates.Add(Index);
		}
	}

	if (Candidates.Num() == 0)
	{
		return nullptr;
	}

	FVector PlayerLocation = FVector::ZeroVector;
	if (const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		PlayerLocation = PlayerPawn->GetActorLocation();
	}

	auto DistSq = [this, &PlayerLocation](int32 Index) -> float
	{
		return FVector::DistSquared(SpawnPoints[Index]->GetActorLocation(), PlayerLocation);
	};

	TArray<int32> Unused;
	for (const int32 Index : Candidates)
	{
		if (!UsedThisWave.Contains(Index))
		{
			Unused.Add(Index);
		}
	}

	TArray<int32>& Pool = Unused.Num() > 0 ? Unused : Candidates;

	if (Pool.Num() > 1)
	{
		int32 ClosestIndex = Pool[0];
		float ClosestDistSq = DistSq(ClosestIndex);
		for (const int32 Index : Pool)
		{
			const float CandidateDistSq = DistSq(Index);
			if (CandidateDistSq < ClosestDistSq)
			{
				ClosestDistSq = CandidateDistSq;
				ClosestIndex = Index;
			}
		}
		Pool.Remove(ClosestIndex);
	}

	int32 ChosenIndex = Pool[0];
	float FarthestDistSq = DistSq(ChosenIndex);
	for (const int32 Index : Pool)
	{
		const float CandidateDistSq = DistSq(Index);
		if (CandidateDistSq > FarthestDistSq)
		{
			FarthestDistSq = CandidateDistSq;
			ChosenIndex = Index;
		}
	}

	UsedThisWave.Add(ChosenIndex);
	return SpawnPoints[ChosenIndex];
}

void AWaveEnemyManager::SpawnEnemy(TSubclassOf<AEnemyCharacter> EnemyClass, AEnemySpawnPoint* SpawnPoint)
{
	UWorld* World = GetWorld();
	if (!World || !EnemyClass)
	{
		return;
	}

	FTransform SpawnTransform = GetActorTransform();
	if (SpawnPoint)
	{
		SpawnTransform = SpawnPoint->GetActorTransform();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s spawning at manager location — no spawn points assigned."), *GetName());
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnParameters.Owner = this;

	AEnemyCharacter* SpawnedEnemy = World->SpawnActor<AEnemyCharacter>(EnemyClass, SpawnTransform, SpawnParameters);
	if (!SpawnedEnemy)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s failed to spawn %s"), *GetName(), *GetNameSafe(EnemyClass));
		return;
	}

	AliveEnemies.Add(SpawnedEnemy);
}
