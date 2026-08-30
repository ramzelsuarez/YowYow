// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SpinningRiotPlayerController.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UInputMappingContext;
class UInputAction;
class UUserWidget;

/**
 * 
 */
UCLASS()
class YOWYOW_API ASpinningRiotPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void EnterTrickMode() const;
	void ExitTrickMode() const;

	UFUNCTION(BlueprintCallable, Category = "Pause")
	void TogglePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "Pause")
	void OpenPauseMenu();

	UFUNCTION(BlueprintCallable, Category = "Pause")
	void OpenPauseMenuOnDeath();

	UFUNCTION(BlueprintCallable, Category = "Pause")
	void ClosePauseMenu();

	UFUNCTION(BlueprintPure, Category = "Pause")
	bool IsPauseMenuOpen() const { return bPauseMenuOpen; }

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void ToggleEnemyAI();
	
protected:
	UPROPERTY(EditAnywhere, Category="Input contexts")
	UInputMappingContext* GameplayIMC = nullptr;

	UPROPERTY(EditAnywhere, Category="Input contexts")
	UInputMappingContext* TrickModeIMC = nullptr;

	UPROPERTY(EditAnywhere, Category = "Pause")
	TObjectPtr<UInputAction> PauseAction;

	UPROPERTY(EditAnywhere, Category = "Pause")
	TSubclassOf<UUserWidget> PauseMenuClass;

	UPROPERTY(EditAnywhere, Category = "Pause", meta = (ClampMin = "0.0"))
	float DeathPauseMenuDelay = 1.25f;

	UPROPERTY(EditAnywhere, Category = "Debug")
	TObjectPtr<UInputAction> ToggleEnemyAIAction;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	UPROPERTY()
	UEnhancedInputLocalPlayerSubsystem* Subsystem = nullptr;

	UPROPERTY()
	TObjectPtr<UUserWidget> PauseMenuWidget;

	bool bPauseMenuOpen = false;

	FTimerHandle DeathPauseMenuTimerHandle;

	bool IsPossessedPawnDead() const;
};
