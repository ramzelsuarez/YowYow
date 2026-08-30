// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/HealthItem.h"

#include "ActorComponents/HealthComponent.h"
#include "Characters/EriCharacter.h"

void AHealthItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AEriCharacter* Eri = Cast<AEriCharacter>(OtherActor);
	if (!Eri)
	{
		return;
	}

	UHealthComponent* HealthComp = Eri->FindComponentByClass<UHealthComponent>();
	if (!HealthComp || HealthComp->IsDead())
	{
		return;
	}

	if (HealthComp->GetCurrentHealth() >= HealthComp->GetMaxHealth())
	{
		return;
	}

	HealthComp->Heal(Health);
	Destroy();
}
