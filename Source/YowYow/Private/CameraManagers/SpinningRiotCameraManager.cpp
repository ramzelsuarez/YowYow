// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraManagers/SpinningRiotCameraManager.h"

void ASpinningRiotCameraManager::UpdateCamera(float DeltaTime)
{
	Super::UpdateCamera(DeltaTime);

	FRotator CurrentRotation = GetCameraRotation();

	if (!CurrentRotation.Equals(GetLastFrameCameraCacheView().Rotation))
	{
		OnCameraRotationChanged.Broadcast(CurrentRotation);
	}
}
