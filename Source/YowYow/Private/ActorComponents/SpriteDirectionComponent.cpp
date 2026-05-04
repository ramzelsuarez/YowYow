// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/SpriteDirectionComponent.h"
#include "CameraManagers/SpinningRiotCameraManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
USpriteDirectionComponent::USpriteDirectionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void USpriteDirectionComponent::BeginPlay()
{
	Super::BeginPlay();

	CameraManager = Cast<ASpinningRiotCameraManager>(UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0));

	if (CameraManager)
	{
		CachedCameraRotation = CameraManager->GetCameraRotation();
		CameraManager->OnCameraRotationChanged.AddUObject(this, &USpriteDirectionComponent::HandleCameraRotationChanged);
	}

	UpdateDirectionFromCamera();
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

void USpriteDirectionComponent::UpdateDirectionFromCamera()
{
	AActor* Owner = GetOwner();

	if (!Owner)
	{
		return;
	}

	const FRotator CameraYawRotation(0.f, CachedCameraRotation.Yaw, 0.f);
	const FVector CameraForward = FRotationMatrix(CameraYawRotation).GetUnitAxis(EAxis::X);
	const FVector CameraRight = FRotationMatrix(CameraYawRotation).GetUnitAxis(EAxis::Y);

	const FVector Velocity = Owner->GetVelocity();
	const FVector HorizontalVelocity(Velocity.X, Velocity.Y, 0.f);
	const bool bIsMoving = HorizontalVelocity.SizeSquared() > FMath::Square(MovementSpeedThreshold);

	if (bIsMoving)
	{
		const FVector MoveDirection = HorizontalVelocity.GetSafeNormal();

		RawDirection = FVector2D(
			FVector::DotProduct(MoveDirection, CameraRight),
			FVector::DotProduct(MoveDirection, CameraForward)
		);
	}
	else
	{
		// While idle, choose the sprite from the camera angle relative to the actor basis so
		// rotating the camera still swaps front/back/left/right and preserves the 2.5D illusion.
		const FVector ViewDirection = (-CameraForward).GetSafeNormal();

		RawDirection = FVector2D(
			FVector::DotProduct(ViewDirection, Owner->GetActorRightVector()),
			FVector::DotProduct(ViewDirection, Owner->GetActorForwardVector())
		);
	}

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

	UpdateDirectionFromCamera();
}
