// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpriteDirectionComponent.generated.h"

class ASpinningRiotCameraManager;

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

	// Calculated cardinal direction for the current sprite selection.
	// X = left/right, Y = front/back.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D Direction = FVector2D::ZeroVector;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void HandleCameraRotationChanged(const FRotator& CameraRotation);
	void UpdateDirectionFromCamera();
	FVector2D QuantizeDirection(const FVector2D& InDirection) const;

	ASpinningRiotCameraManager* CameraManager = nullptr;
	FRotator CachedCameraRotation = FRotator::ZeroRotator;
};
