// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TrickGaugeComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTrickGaugeChanged, float, CurrentGauge, float, MaxGauge);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class YOWYOW_API UTrickGaugeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTrickGaugeComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trick Gauge")
	float MaxGauge = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trick Gauge")
	float CurrentGauge = 0.f;

public:
	UPROPERTY(BlueprintAssignable, Category = "Trick Gauge")
	FOnTrickGaugeChanged OnTrickGaugeChanged;

	UFUNCTION(BlueprintCallable, Category = "Trick Gauge")
	void AddTrickGauge(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Trick Gauge")
	void SpendTrickGauge(float Amount);

	UFUNCTION(BlueprintPure, Category = "Trick Gauge")
	float GetCurrentGauge() const { return CurrentGauge; }

	UFUNCTION(BlueprintPure, Category = "Trick Gauge")
	float GetMaxGauge() const { return MaxGauge; }
};
