// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Jihanki.generated.h"

class AHealthItem;

UCLASS()
class YOWYOW_API AJihanki : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AJihanki();

	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser
	) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drops")
	TSubclassOf<AHealthItem> HealthItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drops")
	FVector DropOffset = FVector(0.f, 0.f, 80.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drops")
	float DropCooldown = 1.0f;

public:	
	float LastDropTime = -999.f;

	void SpawnHealthItem();
};
