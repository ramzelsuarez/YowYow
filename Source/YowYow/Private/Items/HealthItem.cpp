// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/HealthItem.h"

void AHealthItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	// Temporary pickup behavior.
	// Later: find HealthComponent and heal player.
	Destroy();
}
