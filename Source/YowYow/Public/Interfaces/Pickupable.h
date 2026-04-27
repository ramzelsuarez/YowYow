// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Pickupable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPickupable : public UInterface
{
	GENERATED_BODY()
};

/**
 * this interface will exclusively be used by the items
 */
class YOWYOW_API IPickupable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
};
