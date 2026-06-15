// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Homingable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UHomingable : public UInterface
{
	GENERATED_BODY()
};

/**
 * These are things you can use your homing attack (home?) to, that are "homingable".
 * Enemies and I think we should be able to use this for the "grapple" mechanic too 
 */
class YOWYOW_API IHomingable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool GetIsHomingTargeted();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetHomingTargeted(bool bTargeted);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool CanBeHomed() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FVector GetTargetLocation();
};
