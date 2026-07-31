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

	UE_LOG(LogTemp, Warning, TEXT("EnemyAI BeginPlay | Owner: %s | Player: %s | State: %s | WaveManager: %s"),
		OwnerCharacter ? *OwnerCharacter->GetName() : TEXT("NULL"),
		PlayerPawn ? *PlayerPawn->GetName() : TEXT("NULL"),
		StateComponent ? *StateComponent->GetName() : TEXT("NULL"),
		WaveManager ? *WaveManager->GetName() : TEXT("NULL")
	);
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

	// Hitstop: don't override launch with chase movement.
	if (OwnerCharacter->CustomTimeDilation < 0.95f)
	{
		return false;
	}

	// Still riding knockback from LaunchCharacter — leave velocity alone.
	if (KnockbackIgnoreSpeed > 0.f
		&& OwnerCharacter->GetVelocity().SizeSquared2D() > FMath::Square(KnockbackIgnoreSpeed))
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

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	return CurrentTime - LastAttackTime >= AttackCooldown;
}

void UEnemyAIComponent::ReleaseAttackToken()
{
	if (WaveManager && OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s released attack token"), *OwnerCharacter->GetName());
		WaveManager->ReleaseAttackToken(OwnerCharacter);
	}
}

void UEnemyAIComponent::UpdateAI(float DeltaTime)
{
	UE_LOG(LogTemp, Warning, TEXT("Enemy AI Tick"));

	if (!CanAct())
	{
		return;
	}

	FVector EnemyLocation = OwnerCharacter->GetActorLocation();
	FVector PlayerLocation = PlayerPawn->GetActorLocation();

	PlayerLocation.Z = EnemyLocation.Z;

	const float DistanceToPlayer = FVector::Dist(EnemyLocation, PlayerLocation);

	UE_LOG(LogTemp, Warning, TEXT("Distance to player: %f"), DistanceToPlayer);

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
				UE_LOG(LogTemp, Warning, TEXT("%s got attack token"), *OwnerCharacter->GetName());

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

	UE_LOG(LogTemp, Warning, TEXT("%s moving toward player"), *OwnerCharacter->GetName());

	OwnerCharacter->AddActorWorldOffset(MoveDelta, true);
}