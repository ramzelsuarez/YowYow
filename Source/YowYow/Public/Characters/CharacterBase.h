// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "Interfaces/Damageable.h"
#include "Types/AttackTypes.h"
#include "CharacterBase.generated.h"

class UCharacterStateComponent;
class ASpinningRiotCameraManager;
class USpriteDirectionComponent;
class UHealthComponent;
class UAttackComponent;
class UCharacterAttackData;
class UCameraShakeBase;

/**
 * this class is the base for all characters in the game, it should implement the following components: (besides obvious movement component etc)
 * - UPaperZDAnimationComponent: self explanatory and needed for all APaperZDCharacter children
 * - UHealthComponent / UAttackComponent / UCharacterStateComponent: created in C++ on this class.
 *   Do not add extra copies on the Blueprint (duplicates).
 * - USpriteDirectionComponent: component that will cache the player's camera, get its direction, compare it to this character's forward
 *		and generate sprite directionality based on it. The PaperZD ABP should consume from it to display the correct sprite
 * - UCharacterStateComponent: it dictates general states that will help us determine if certain actions can be performed at certain points
 *		Example: you can't move while attacking; air light/heavy are allowed; homing is a separate input; etc.
 *		Try to keep the states simple. For more meticulous states like stages of attacking (start, active, canCombo, recovery, etc) we can calculate that, for example,
 *		in the attack component. These are super basic general states to determine if a character can perform certain actions.
 */
UCLASS()
class YOWYOW_API ACharacterBase : public APaperZDCharacter, public IDamageable
{
	GENERATED_BODY()

public:
	ACharacterBase();

	virtual void Tick(float DeltaTime) override;

	virtual float TakeDamage(
		float DamageAmount,
		FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser
	) override;

	/*
	 * Movement actions
	 */
	void DoMove(float Right, float Forward);

	/**
	 * Attack actions
	 */
	// default attack is normal since it's the most common usage and the only one enemies will use (probably)
	UFUNCTION(BlueprintCallable, Category="Combat")
	bool DoAttack(EAttackType AttackType = EAttackType::Normal);

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsDead() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat")
	UCharacterAttackData* AttackData;

	/** Player-only hurt feedback. Assign a CameraShake class in the BP. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Feedback")
	TSubclassOf<UCameraShakeBase> DamageCameraShake;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Feedback", meta=(ClampMin="0.0"))
	float DamageCameraShakeScale = 1.f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Jump() override;
	virtual void StopJumping() override;
	
	bool CanMove();

	UPROPERTY()
	ASpinningRiotCameraManager* CameraManager = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UAttackComponent* AttackComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UHealthComponent* HealthComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCharacterStateComponent* CharacterStateComponent = nullptr;

	/**
	 * Ana confirmed enemies will have only 1 direction (always face the screen).
	 * I trust that the graphics team will realize how shit that looks once it's implemented,
	 * that they will be left with no other option but to provide us with
	 * full enemy directionality. (at the cost of their sleeping hours)
	 * That's the reason SpriteDirectionComponent lays in here instead of only on the Eri class.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USpriteDirectionComponent* SpriteDirectionComponent = nullptr;

private:
	void HandleCameraRotationChanged(const FRotator &CameraRotation);

	UFUNCTION()
	void HandleMovementModeChanged(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode);
	
	UFUNCTION()
	void HandleLanded(const FHitResult& Hit);

	UFUNCTION()
	void HandleHealthDepleted(UHealthComponent* InHealthComponent, AActor* DamageCauser);

	UFUNCTION()
	void HandleHealthDamageTaken(
		UHealthComponent* InHealthComponent,
		int32 Damage,
		int32 InCurrentHealth,
		AActor* DamageCauser,
		AController* DamageInstigator
	);
};
