#include "ActorComponents/EnemyAIComponent.h"

#include "ActorComponents/CharacterStateComponent.h"
#include "Characters/CharacterBase.h"
#include "CharacterStates/CharacterStates.h"
#include "Kismet/GameplayStatics.h"
#include "BattleSystem/WaveEnemyManager.h"
#include "TimerManager.h"

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

	if (!WaveManager)
	{
		WaveManager = Cast<AWaveEnemyManager>(
			UGameplayStatics::GetActorOfClass(GetWorld(), AWaveEnemyManager::StaticClass())
		);
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

void UEnemyAIComponent::ReleaseAttackToken()
{
	{
		if (WaveManager && OwnerCharacter)
		{
			WaveManager->ReleaseAttackToken(OwnerCharacter);
		}
	}
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
			if (!WaveManager || WaveManager->RequestAttackToken(OwnerCharacter))
			{
				LastAttackTime = GetWorld()->GetTimeSeconds();
				OwnerCharacter->DoAttack(EAttackType::Normal);

				GetWorld()->GetTimerManager().ClearTimer(AttackTokenReleaseTimerHandle);
				GetWorld()->GetTimerManager().SetTimer(
					AttackTokenReleaseTimerHandle,
					this,
					&UEnemyAIComponent::ReleaseAttackToken,
					TokenReleaseDelay,
					false
				);
			}
		}

		return;
	}
	const FVector Direction = (PlayerLocation - EnemyLocation).GetSafeNormal();
	const FVector MoveDelta = Direction * MoveSpeed * DeltaTime;

	OwnerCharacter->AddActorWorldOffset(MoveDelta, true);
}