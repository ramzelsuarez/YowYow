// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Pickupable.h"
#include "ItemBase.generated.h"

/*
* The base class for items. We should only have two items, an hp one and a trick gauge one. Those will be children of this one.
* Besides implementing the pickupable interface, it should implement a component to always face to the camera, similar to what ACharacterBase does
*/
UCLASS()
class YOWYOW_API AItemBase : public AActor, public IPickupable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
