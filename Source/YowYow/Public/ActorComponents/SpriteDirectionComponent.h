// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpriteDirectionComponent.generated.h"

/**
 * This component is supposed to calculate rotation relative to the player (active player controller) owning camera
 * and then update Direction value.
 * The value should be read by the  ABP for the character using this component and update its directionality
 * thus, displaying the correct sprite on screen
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YOWYOW_API USpriteDirectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USpriteDirectionComponent();

	// The direction of the sprite (x and y)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Direction;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// this should be called every tick so the sprites for the owning character are always updated.
	// we COULD optimize to perform the calculation only on camera rotation, but that would require a lot of work
	// maybe if we have time to spare
	void UpdateDirectionFromCamera();
};
