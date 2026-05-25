// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/HealthItem.h"
#include "Interfaces/Pickupable.h"
#include "Items/ItemBase.h"



void AHealthItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	IPickupable* Pickupable = Cast<IPickupable>(OtherActor);
	if (Pickupable)
	{
		//Pickupable->AddHealth(this);

		//SpawnPickupSystem();
		//SpawnPickupSound();

		Destroy();
	}

}
