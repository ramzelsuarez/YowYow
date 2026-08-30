// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/SpriteDirectionComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "CameraManagers/SpinningRiotCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

USpriteDirectionComponent::USpriteDirectionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void USpriteDirectionComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(true);
	TryBindCameraManager();
	UpdateDirectionFromCamera();
}

void USpriteDirectionComponent::TryBindCameraManager()
{
	if (CameraManager)
	{
		return;
	}

	APlayerCameraManager* AnyCamera = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	CameraManager = Cast<ASpinningRiotCameraManager>(AnyCamera);
	if (!CameraManager)
	{
		return;
	}

	CachedCameraRotation = CameraManager->GetCameraRotation();
	CameraManager->OnCameraRotationChanged.AddUObject(this, &USpriteDirectionComponent::HandleCameraRotationChanged);
}

void USpriteDirectionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CameraManager)
	{
		CameraManager->OnCameraRotationChanged.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void USpriteDirectionComponent::HandleCameraRotationChanged(const FRotator& CameraRotation)
{
	CachedCameraRotation = CameraRotation;
	UpdateDirectionFromCamera();
}

FRotator USpriteDirectionComponent::ResolveCameraRotation() const
{
	if (APlayerCameraManager* Cam = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0))
	{
		return Cam->GetCameraRotation();
	}

	if (const APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		return PC->GetControlRotation();
	}

	return CachedCameraRotation;
}

void USpriteDirectionComponent::UpdateDirectionFromCamera()
{
	AActor* Owner = GetOwner();

	if (!Owner)
	{
		return;
	}

	CachedCameraRotation = ResolveCameraRotation();

	const FRotator CameraYawRotation(0.f, CachedCameraRotation.Yaw, 0.f);
	const FVector CameraForward = FRotationMatrix(CameraYawRotation).GetUnitAxis(EAxis::X);
	const FVector CameraRight = FRotationMatrix(CameraYawRotation).GetUnitAxis(EAxis::Y);
	const FVector OwnerForward = Owner->GetActorForwardVector();

	FVector2D RawDirection = FVector2D(
		FVector::DotProduct(OwnerForward, CameraRight),
		// make forward slightly smaller so that side sprites are dominant, but check later which one looks better 
		FVector::DotProduct(OwnerForward, CameraForward * 0.999f)
	);

	Direction = QuantizeDirection(RawDirection);
}

// Direction = (1, 0): Right
// Direction = (-1, 0): Left
// Direction = (0, 1): Front (forward facing)
// Direction = (0, -1): Back (character faces to the camera)
FVector2D USpriteDirectionComponent::QuantizeDirection(const FVector2D& InDirection) const
{
	if (InDirection.IsNearlyZero())
	{
		return Direction;
	}

	if (FMath::Abs(InDirection.X) >= FMath::Abs(InDirection.Y))
	{
		return FVector2D(FMath::Sign(InDirection.X), 0.f);
	}

	return FVector2D(0.f, FMath::Sign(InDirection.Y));
}


// Called every frame
void USpriteDirectionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TryBindCameraManager();
	UpdateDirectionFromCamera();
}
