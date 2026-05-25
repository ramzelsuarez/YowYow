// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/ItemBase.h"
#include "Components/SphereComponent.h"
#include "HealthItem.generated.h"

class UNiagaraComponent;


UCLASS()
class YOWYOW_API AHealthItem : public AItemBase, public IPickupable
{
	GENERATED_BODY()
protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
private:
	UPROPERTY(EditAnywhere, Category = Items)
	int32 Health;
public:
	FORCEINLINE int32 GetHealth() const { return Health; }
	FORCEINLINE void SetHealth(int32 NumberOfHealth) { Health = NumberOfHealth; }

	//virtual void Pickup_Execute();

	/*AealthItem::Pickup_Execute() {
		Destroy();

		GrantEffect(EItemEffects::RecoverHealth)


			asdasdsa::GrantEffect(EItemEffect* Effect) {
			AEriCharacter* Target->ReceiveEffects(Effect) ZEL deliv - work on the item pickup system*/
};
