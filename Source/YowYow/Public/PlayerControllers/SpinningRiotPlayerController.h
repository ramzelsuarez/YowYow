// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SpinningRiotPlayerController.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UInputMappingContext;

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
	
protected:
	UPROPERTY(EditAnywhere, Category="Input contexts")
	UInputMappingContext* GameplayIMC = nullptr;

	UPROPERTY(EditAnywhere, Category="Input contexts")
	UInputMappingContext* TrickModeIMC = nullptr;

	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UEnhancedInputLocalPlayerSubsystem* Subsystem = nullptr;
};
