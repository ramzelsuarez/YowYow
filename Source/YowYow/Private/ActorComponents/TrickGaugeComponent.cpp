// Fill out your copyright notice in the Description page of Project Settings.
// Temporary simple gauge implementation for item pickup testing.
// To Franco: you can replace/expand this later with drain, trick mode logic, UI, etc. hehe - Zellybananalicioso


#include "ActorComponents/TrickGaugeComponent.h"

UTrickGaugeComponent::UTrickGaugeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTrickGaugeComponent::BeginPlay()
{
	Super::BeginPlay();

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

