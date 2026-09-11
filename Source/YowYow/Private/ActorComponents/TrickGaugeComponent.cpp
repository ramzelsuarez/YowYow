// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/TrickGaugeComponent.h"

UTrickGaugeComponent::UTrickGaugeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTrickGaugeComponent::BeginPlay()
{
	Super::BeginPlay();

	MaxGauge = FMath::Max(MaxGauge, 0.f);
	CurrentGauge = FMath::Clamp(CurrentGauge, 0.f, MaxGauge);
	OnTrickGaugeChanged.Broadcast(CurrentGauge, MaxGauge);
}

void UTrickGaugeComponent::AddTrickGauge(float Amount)
{
	if (Amount <= 0.f)
	{
		return;
	}

	CurrentGauge = FMath::Clamp(CurrentGauge + Amount, 0.f, MaxGauge);
	OnTrickGaugeChanged.Broadcast(CurrentGauge, MaxGauge);
}

void UTrickGaugeComponent::SpendTrickGauge(float Amount)
{
	if (Amount <= 0.f)
	{
		return;
	}

	CurrentGauge = FMath::Clamp(CurrentGauge - Amount, 0.f, MaxGauge);
	OnTrickGaugeChanged.Broadcast(CurrentGauge, MaxGauge);
}

void UTrickGaugeComponent::FillFromHit()
{
	AddTrickGauge(GaugePerHit);
}

void UTrickGaugeComponent::Drain(float DeltaTime)
{
	SpendTrickGauge(FMath::Max(0.f, DrainPerSecond) * FMath::Max(0.f, DeltaTime));
}

void UTrickGaugeComponent::EmptyGauge()
{
	CurrentGauge = 0.f;
	OnTrickGaugeChanged.Broadcast(CurrentGauge, MaxGauge);
}

