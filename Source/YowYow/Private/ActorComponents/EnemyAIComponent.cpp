#include "ActorComponents/EnemyAIComponent.h"

#include "ActorComponents/CharacterStateComponent.h"
#include "Characters/CharacterBase.h"
#include "CharacterStates/CharacterStates.h"
#include "Kismet/GameplayStatics.h"

UEnemyAIComponent::UEnemyAIComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEnemyAIComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacterBase>(GetOwner());
	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	
	if (OwnerCharacter)
	{
		StateComponent = OwnerCharacter->FindComponentByClass<UCharacterStateComponent>();
	}
}

void UEnemyAIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateAI(DeltaTime);
}

bool UEnemyAIComponent::CanAct() const
{
	if (!OwnerCharacter || !PlayerPawn || !StateComponent)
	{
		return false;
	}

	if (StateComponent->GetLifeState() == ECharacterLifeState::Dead)
	{
		return false;
	}

	if (StateComponent->GetActionState() == ECharacterActionState::Attacking)
	{
		return false;
	}

	return true;
}

bool UEnemyAIComponent::CanAttack() const
{
	if (!CanAct())
	{
		return false;
	}

	if (StateComponent->GetLocomotionState() == ECharacterLocomotionState::Airborne)
	{
		return false;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	return CurrentTime - LastAttackTime >= AttackCooldown;
}

void UEnemyAIComponent::UpdateAI(float DeltaTime)
{
	if (!CanAct())
	{
		return;
	}

	FVector EnemyLocation = OwnerCharacter->GetActorLocation();
	FVector PlayerLocation = PlayerPawn->GetActorLocation();

	if (bIgnoreZ)
	{
		PlayerLocation.Z = EnemyLocation.Z;
	}

	const float DistanceToPlayer = FVector::Dist(EnemyLocation, PlayerLocation);

	if (DistanceToPlayer > DetectionRange)
	{
		return;
	}

	if (DistanceToPlayer <= AttackRange)
	{
		if (CanAttack())
		{
			LastAttackTime = GetWorld()->GetTimeSeconds();
			OwnerCharacter->DoAttack(EAttackType::Normal);
		}

		return;
	}

	const FVector Direction = (PlayerLocation - EnemyLocation).GetSafeNormal();
	const FVector MoveDelta = Direction * MoveSpeed * DeltaTime;

	OwnerCharacter->AddActorWorldOffset(MoveDelta, true);
}