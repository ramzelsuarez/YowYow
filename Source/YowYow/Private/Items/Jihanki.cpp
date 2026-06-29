// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Jihanki.h"
#include "Items/HealthItem.h"

// Sets default values
AJihanki::AJihanki()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
}

float AJihanki::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	SpawnHealthItem();
	return DamageAmount;
}

void AJihanki::SpawnHealthItem()
{
	if (!HealthItemClass || !GetWorld())
	{
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();

	if (CurrentTime - LastDropTime < DropCooldown)
	{
		return;
	}

	LastDropTime = CurrentTime;

	const FVector SpawnLocation = GetActorLocation() + DropOffset;
	const FRotator SpawnRotation = GetActorRotation();

	GetWorld()->SpawnActor<AHealthItem>(HealthItemClass, SpawnLocation, SpawnRotation);
}