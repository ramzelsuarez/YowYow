// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/ItemBase.h"
#include "TrickGaugeItem.generated.h"

/**
 * 
 */
UCLASS()
class YOWYOW_API ATrickGaugeItem : public AItemBase
{
	GENERATED_BODY()

protected:
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	) override;
private:
	UPROPERTY(EditAnywhere, Category = "Items")
	float TrickGaugeAmount = 25.f;

public:
	FORCEINLINE float GetTrickGaugeAmount() const { return TrickGaugeAmount; }
	
};
