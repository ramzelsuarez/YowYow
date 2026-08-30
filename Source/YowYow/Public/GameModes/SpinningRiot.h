// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SpinningRiot.generated.h"

class AWaveEnemyManager;

UENUM(BlueprintType)
enum class EDemoPhase : uint8
{
	Combat UMETA(DisplayName = "Combat"),
	Puzzle UMETA(DisplayName = "Puzzle"),
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

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void SetDemoPhase(EDemoPhase NewPhase);

	UFUNCTION()
	void HandleEncounterCompleted();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Demo")
	EDemoPhase DemoPhase = EDemoPhase::Combat;

	UPROPERTY()
	TObjectPtr<AWaveEnemyManager> WaveManager;
};
