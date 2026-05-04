// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "SpinningRiotCameraManager.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(
	FOnCameraRotationChanged,
	const FRotator&
)
/**
 * Custom camera manager that will broadcast the camera rotation to any subscribers.
 * This is used to make sprites always face the camera.
 */
UCLASS()
class YOWYOW_API ASpinningRiotCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	FOnCameraRotationChanged OnCameraRotationChanged;

protected:
	virtual void UpdateCamera(float DeltaTime) override;
};
