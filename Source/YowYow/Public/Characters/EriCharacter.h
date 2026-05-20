// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase.h"
#include "EriCharacter.generated.h"

struct FInputActionValue;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;
class UHomingAttackComponent;
class UTrickGaugeComponent;
/**
 * This class represents the player character, Eri. We shall pressure planners to never change the name of the main character,
 * since our code is now sentenced to have this class named like this.
 *
 * This class should implement the following components: (besides camera and spring arm)
 * - UTrickGaugeComponent: only manages the trick gauge and its drain/fill
 * - UYoyoTrickComponent: should handle the "trick mode" and its inputs
 * - UHomingAttackComponent: self explanatory (check documentation if not sure about what's a homing attack)
 *
 * Eri may or may not require a custom PaperZDAnimInstance component (which will later be instantiated as a BP)
 * the reason is that it's much easier to read code than blueprint spaghetti when it comes to calculating all that stuff
 * and the BP would only "read" the variables in there without any of the ugly stuff ^^
 */
UCLASS()
class YOWYOW_API AEriCharacter : public ACharacterBase
{
	GENERATED_BODY()

public:
	AEriCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;

	/**
	 * Input Actions 
	 */
	UPROPERTY(EditAnywhere, Category="Input Actions|Movement")
	UInputAction* MovementAction = nullptr;
	
	UPROPERTY(EditAnywhere, Category="Input Actions|Movement")
	UInputAction* JumpAction = nullptr;
	
	UPROPERTY(EditAnywhere, Category="Input Actions|Movement")
	UInputAction* LookAction = nullptr;

	UPROPERTY(EditAnywhere, Category="Input Actions|Combat")
	UInputAction* AttackAction = nullptr;

	UPROPERTY(EditAnywhere, Category="Input Actions|Combat")
	UInputAction* AreaAttackAction = nullptr;

	UPROPERTY(EditAnywhere, Category="Input Actions|Trick")
	UInputAction* TrickModeAction = nullptr;

	UPROPERTY(EditAnywhere, Category="Input Actions|Trick")
	UInputAction* TrickInputAction = nullptr;

	/**
	 * Input callbacks
	 */
	void Move(const FInputActionValue& Value);
	void JumpPressed();
	void JumpReleased();
	void Look(const FInputActionValue& Value);
	void TryAttack();
	void TryAreaAttack();
	void EnterTrickMode();
	void ExitTrickMode();
	void TryTrickInput(const FInputActionValue& Value);
	
	/**
	 * Actor Components
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UTrickGaugeComponent* TrickGaugeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UHomingAttackComponent* HomingAttackComponent;

	/**
	 * Actor Component events
	 */
	UFUNCTION()
	void HandleHomingAttackFinished(const bool bSuccess);

	/**
	 * Camera
	 */
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;
};
