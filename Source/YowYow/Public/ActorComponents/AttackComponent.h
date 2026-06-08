// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttackComponent.generated.h"

/**
 * This component is meant to be used by both Eri and enemies
 *
 * it should handle:
 * - Setting the character state to attacking
 * - Checking for combo chains (a Data Asset should provide info on whether there are combo attacks left or not etc)
 * - Spawning hitboxes (a Data Asset should provide info on how and where they are spawned)
 * - Optional: Tell the Character class that it should play an animation.
 *			   It's optional because maybe the character will handle this in ABP (as opposed to playing an AnimMontage).
 *			   Not sure how it's done with PaperZD 
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YOWYOW_API UAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAttackComponent();
	
	UFUNCTION()
	void TryAttack();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
