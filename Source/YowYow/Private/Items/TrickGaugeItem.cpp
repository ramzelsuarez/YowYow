// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/TrickGaugeItem.h"
#include "ActorComponents/TrickGaugeComponent.h"
#include "Kismet/GameplayStatics.h"

void ATrickGaugeItem::OnSphereOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	UTrickGaugeComponent* TrickGauge = OtherActor->FindComponentByClass<UTrickGaugeComponent>();

	if (TrickGauge)
	{
		TrickGauge->AddTrickGauge(TrickGaugeAmount);
		Destroy();
	}
}
