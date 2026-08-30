#pragma once

#include "CoreMinimal.h"
#include "Containers/Set.h"
#include "GameFramework/Actor.h"
#include "WaveEnemyManager.generated.h"

class AActor;
class AEnemyCharacter;
class AEnemySpawnPoint;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEncounterCompleted);

USTRUCT(BlueprintType)
struct FEnemyWaveEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Encounter")
	TSubclassOf<AEnemyCharacter> EnemyClass;

	/** 0..n into SpawnPoints, or -1 to pick automatically (unused path, not closest to player). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Encounter", meta = (ClampMin = "-1"))
	int32 SpawnPointIndex = -1;
};

USTRUCT(BlueprintType)
struct FEnemyWave
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Encounter")
	TArray<FEnemyWaveEntry> Entries;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Encounter", meta = (ClampMin = "0.0"))
	float DelayBeforeWave = 1.5f;
};

UCLASS()
class YOWYOW_API AWaveEnemyManager : public AActor
{
	GENERATED_BODY()

public:
	AWaveEnemyManager();

	UFUNCTION(BlueprintCallable, Category = "Enemy Wave")
	bool RequestAttackToken(AActor* RequestingEnemy);

	UFUNCTION(BlueprintCallable, Category = "Enemy Wave")
	void ReleaseAttackToken(AActor* ReleasingEnemy);

	UFUNCTION(BlueprintCallable, Category = "Enemy Wave")
	void RegisterEnemyDefeated(AActor* DefeatedEnemy);

	UFUNCTION(BlueprintCallable, Category = "Encounter")
	void StartEncounter();

	UFUNCTION(BlueprintPure, Category = "Encounter")
	bool IsEncounterCompleted() const { return bEncounterCompleted; }

	UFUNCTION(BlueprintPure, Category = "Encounter")
	bool IsEncounterStarted() const { return bEncounterStarted; }

	UPROPERTY(BlueprintAssignable, Category = "Encounter")
	FOnEncounterCompleted OnEncounterCompleted;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Encounter")
	bool bAutoStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Encounter", meta = (ClampMin = "0.0", EditCondition = "bAutoStart"))
	float EncounterStartDelay = 1.f;

	/** Used when a wave entry has no class set. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Encounter")
	TSubclassOf<AEnemyCharacter> DefaultEnemyClass;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Encounter")
	TArray<TObjectPtr<AEnemySpawnPoint>> SpawnPoints;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Encounter")
	TArray<FEnemyWave> Waves;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Wave")
	int32 MaxAttackTokens = 2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy Wave")
	int32 DefeatedEnemies = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Encounter")
	int32 CurrentWaveIndex = -1;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> EnemiesWithAttackTokens;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> AliveEnemies;

private:
	bool bEncounterStarted = false;
	bool bEncounterCompleted = false;

	FTimerHandle EncounterStartTimerHandle;
	FTimerHandle SpawnWaveTimerHandle;

	void AdvanceWave();
	void SpawnCurrentWave();
	void CompleteEncounter();

	TSubclassOf<AEnemyCharacter> ResolveEnemyClass(const FEnemyWaveEntry& Entry) const;
	AEnemySpawnPoint* PickSpawnPoint(int32 RequestedIndex, TSet<int32>& UsedThisWave) const;
	void SpawnEnemy(TSubclassOf<AEnemyCharacter> EnemyClass, AEnemySpawnPoint* SpawnPoint);
};
