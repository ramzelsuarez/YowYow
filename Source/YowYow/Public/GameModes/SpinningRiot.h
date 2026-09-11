// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Types/ComboTypes.h"
#include "SpinningRiot.generated.h"

class AWaveEnemyManager;
class UUserWidget;

UENUM(BlueprintType)
enum class EDemoPhase : uint8
{
	Combat UMETA(DisplayName = "Combat"),
	Puzzle UMETA(DisplayName = "Puzzle (Deprecated)"), // Retained serialized value; never entered.
	Tutorial UMETA(DisplayName = "Tutorial"),
	GameWon UMETA(DisplayName = "Game Won"),
	GameOver UMETA(DisplayName = "Game Over"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnDemoPhaseChanged,
	EDemoPhase, OldPhase,
	EDemoPhase, NewPhase
);

/**
 * This is the GameMode for the actual game, like, when you're playing (as opposed to the main menu one).
 * Main difference being the player controller employed in this one
 */
UCLASS()
class YOWYOW_API ASpinningRiot : public AGameModeBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Demo")
	EDemoPhase GetDemoPhase() const { return DemoPhase; }

	UPROPERTY(BlueprintAssignable, Category = "Demo")
	FOnDemoPhaseChanged OnDemoPhaseChanged;

	UFUNCTION(BlueprintCallable, Category = "Demo")
	void FinishTutorial();

	UFUNCTION(BlueprintCallable, Category = "Demo")
	void HandlePlayerDied();

	UFUNCTION(BlueprintPure, Category = "Demo|Results")
	EComboTier GetWonComboTier() const { return WonComboTier; }

	UFUNCTION(BlueprintPure, Category = "Demo|Results")
	int32 GetWonComboHits() const { return WonComboHits; }

	UFUNCTION(BlueprintPure, Category = "Demo|Results")
	int32 GetWonComboPoints() const { return WonComboPoints; }

	void RefreshDemoInput();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void SetDemoPhase(EDemoPhase NewPhase);

	UFUNCTION()
	void HandleEncounterCompleted();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Demo")
	EDemoPhase DemoPhase = EDemoPhase::Tutorial;

	UPROPERTY()
	TObjectPtr<AWaveEnemyManager> WaveManager;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Demo|UI")
	TSubclassOf<UUserWidget> TutorialWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Demo|UI")
	TSubclassOf<UUserWidget> GameWonWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Demo|UI")
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> ActivePhaseWidget;

private:
	void CompleteDemoWin();
	EComboTier WonComboTier = EComboTier::None;
	int32 WonComboHits = 0;
	int32 WonComboPoints = 0;
	bool bDemoPhaseInitialized = false;
	FTimerHandle DemoWinTimer;
};
