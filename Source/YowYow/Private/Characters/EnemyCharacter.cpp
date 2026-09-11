// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/EnemyCharacter.h"

#include "ActorComponents/CharacterStateComponent.h"
#include "ActorComponents/HealthComponent.h"
#include "BattleSystem/WaveEnemyManager.h"
#include "CharacterStates/CharacterStates.h"
#include "Items/HealthItem.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Components/SceneComponent.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
	HomingTargetMarker = CreateDefaultSubobject<USceneComponent>(TEXT("HomingTargetMarker"));
	HomingTargetMarker->SetupAttachment(GetRootComponent());
	HomingTargetMarker->SetRelativeLocation(FVector(0.f, 0.f, 80.f));
	HomingMarkerWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HomingMarkerWidget"));
	HomingMarkerWidget->SetupAttachment(HomingTargetMarker);
	HomingMarkerWidget->SetWidgetSpace(EWidgetSpace::Screen);
	HomingMarkerWidget->SetDrawAtDesiredSize(true);
	HomingMarkerWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemyCharacter::PostInitializeComponents()
{
	// Our component drives AI without a controller. CharacterMovement must still
	// initialize and simulate movement for LaunchCharacter to apply knockback.
	GetCharacterMovement()->bRunPhysicsWithNoController = true;
	Super::PostInitializeComponents();
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	HomingMarkerWidget->SetWidgetClass(HomingMarkerWidgetClass);
	SetHomingTargeted_Implementation(false);

	if (HealthComponent)
	{
		HealthComponent->OnHealthDepleted.AddDynamic(this, &AEnemyCharacter::HandleEnemyHealthDepleted);
	}

	if (!WaveManager)
	{
		WaveManager = Cast<AWaveEnemyManager>(
			UGameplayStatics::GetActorOfClass(GetWorld(), AWaveEnemyManager::StaticClass())
		);
	}
}

void AEnemyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CorpseDestroyTimerHandle);
	}

	if (HealthComponent)
	{
		HealthComponent->OnHealthDepleted.RemoveDynamic(this, &AEnemyCharacter::HandleEnemyHealthDepleted);
	}

	Super::EndPlay(EndPlayReason);
}

bool AEnemyCharacter::GetIsHomingTargeted_Implementation()
{
	return bIsHomingTargeted;
}

void AEnemyCharacter::SetHomingTargeted_Implementation(bool bTargeted)
{
	bIsHomingTargeted = bTargeted;
	if (HomingTargetMarker)
	{
		HomingTargetMarker->SetHiddenInGame(!bTargeted, true);
		HomingTargetMarker->SetVisibility(bTargeted, true);
	}
}

bool AEnemyCharacter::CanBeHomed_Implementation() const
{
	if (HealthComponent && HealthComponent->IsDead())
	{
		return false;
	}

	if (CharacterStateComponent && CharacterStateComponent->GetLifeState() == ECharacterLifeState::Dead)
	{
		return false;
	}

	return true;
}

bool AEnemyCharacter::CanGrantCombo_Implementation() const
{
	if (HealthComponent && HealthComponent->IsDead())
	{
		return false;
	}

	if (CharacterStateComponent && CharacterStateComponent->GetLifeState() == ECharacterLifeState::Dead)
	{
		return false;
	}

	return true;
}

FVector AEnemyCharacter::GetTargetLocation_Implementation()
{
	return GetActorLocation();
}

void AEnemyCharacter::HandleEnemyHealthDepleted(UHealthComponent* InHealthComponent, AActor* DamageCauser)
{
	SetHomingTargeted_Implementation(false);
	if (WaveManager)
	{
		WaveManager->RegisterEnemyDefeated(this);
	}

	TryDropHealthItem();

	UWorld* World = GetWorld();
	if (!World)
	{
		Destroy();
		return;
	}

	if (CorpseLifetime <= 0.f)
	{
		Destroy();
		return;
	}

	World->GetTimerManager().SetTimer(
		CorpseDestroyTimerHandle,
		this,
		&AEnemyCharacter::DestroyCorpse,
		CorpseLifetime,
		false
	);
}

void AEnemyCharacter::TryDropHealthItem()
{
	if (!HealthItemClass || HealthDropChance <= 0.f)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || FMath::FRand() >= HealthDropChance)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	World->SpawnActor<AHealthItem>(
		HealthItemClass,
		GetActorLocation() + HealthDropOffset,
		GetActorRotation(),
		SpawnParams
	);
}

void AEnemyCharacter::DestroyCorpse()
{
	Destroy();
}
