// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/ItemBase.h"
#include "HealthItem.generated.h"

/**
 * 
 */
UCLASS()
class YOWYOW_API AHealthItem : public AItemBase
{
	GENERATED_BODY()
protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
private:
	UPROPERTY(EditAnywhere, Category = Items)
	int32 Health;
public:
	FORCEINLINE int32 GetHealth() const { return Health;  }
};
