// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "Interfaces/Damageable.h"
#include "CharacterBase.generated.h"

class ASpinningRiotCameraManager;
class USpriteDirectionComponent;
class UHealthComponent;
class UAttackComponent;

/**
 * this class is the base for all characters in the game, it should implement the following components: (besides obvious movement component etc)
 * - UPaperZDAnimationComponent: self explanatory and needed for all APaperZDCharacter children
 * - UHealthComponent: self explanatory / avoid if we have npcs (not sure if we will have them, but those don't engage in combat  and that would make this base character even simpler)
 * - UAttackComponent: self explanatory / avoid if we have npcs (not sure if we will have them, but those don't attack and that would make this base character even simpler)
 * - USpriteDirectionComponent: component that will cache the player's camera, get its direction, compare it to this character's forward
 *		and generate sprite directionality based on it. The PaperZD ABP should consume from it to display the correct sprite
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

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Jump() override;
	virtual void StopJumping() override;

	ASpinningRiotCameraManager* CameraManager = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UAttackComponent* AttackComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UHealthComponent* HealthComponent = nullptr;

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
};
