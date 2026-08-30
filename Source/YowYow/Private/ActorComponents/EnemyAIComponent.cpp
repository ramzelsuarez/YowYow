#include "ActorComponents/EnemyAIComponent.h"

#include "ActorComponents/CharacterStateComponent.h"
#include "Characters/CharacterBase.h"
#include "CharacterStates/CharacterStates.h"
#include "Kismet/GameplayStatics.h"
#include "BattleSystem/WaveEnemyManager.h"
#include "TimerManager.h"

bool UEnemyAIComponent::bGlobalAIFrozen = false;

UEnemyAIComponent::UEnemyAIComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEnemyAIComponent::SetGlobalAIFrozen(bool bFrozen)
{
	if (bGlobalAIFrozen == bFrozen)
	{
		return;
	}

	bGlobalAIFrozen = bFrozen;
	UE_LOG(LogTemp, Warning, TEXT("Enemy AI %s"), bGlobalAIFrozen ? TEXT("FROZEN") : TEXT("RUNNING"));
}

void UEnemyAIComponent::ToggleGlobalAIFrozen()
{
	SetGlobalAIFrozen(!bGlobalAIFrozen);
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

	if (bGlobalAIFrozen)
	{
		return;
	}

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
		WaveManager->ReleaseAttackToken(OwnerCharacter);
	}
}

void UEnemyAIComponent::FacePlayer()
{
	if (!OwnerCharacter || !PlayerPawn)
	{
		return;
	}

	FVector ToPlayer = PlayerPawn->GetActorLocation() - OwnerCharacter->GetActorLocation();
	ToPlayer.Z = 0.f;
	if (!ToPlayer.Normalize())
	{
		return;
	}

	OwnerCharacter->SetActorRotation(ToPlayer.Rotation());
}

void UEnemyAIComponent::UpdateAI(float DeltaTime)
{
	if (!CanAct())
	{
		return;
	}

	FVector EnemyLocation = OwnerCharacter->GetActorLocation();
	FVector PlayerLocation = PlayerPawn->GetActorLocation();
	PlayerLocation.Z = EnemyLocation.Z;

	const float DistanceToPlayer = FVector::Dist(EnemyLocation, PlayerLocation);
	if (DistanceToPlayer > DetectionRange)
	{
		return;
	}

	FacePlayer();

	if (DistanceToPlayer <= AttackRange)
	{
		if (CanAttack())
		{
			const bool bHasToken = !WaveManager || WaveManager->RequestAttackToken(OwnerCharacter);
			if (bHasToken)
			{
				if (OwnerCharacter->DoAttack(EAttackType::Normal))
				{
					LastAttackTime = GetWorld()->GetTimeSeconds();
					GetWorld()->GetTimerManager().ClearTimer(AttackTokenReleaseTimerHandle);
					GetWorld()->GetTimerManager().SetTimer(
						AttackTokenReleaseTimerHandle,
						this,
						&UEnemyAIComponent::ReleaseAttackToken,
						TokenReleaseDelay,
						false
					);
				}
				else
				{
					ReleaseAttackToken();
					if (!bLoggedMissingAttackSetup)
					{
						bLoggedMissingAttackSetup = true;
						UE_LOG(
							LogTemp,
							Warning,
							TEXT("%s DoAttack failed — check AttackData.Normal (ArcSweep) on the enemy."),
							*OwnerCharacter->GetName()
						);
					}
				}
			}
		}

		return;
	}

	const FVector Direction = (PlayerLocation - EnemyLocation).GetSafeNormal();
	OwnerCharacter->AddActorWorldOffset(Direction * MoveSpeed * DeltaTime, true);
}