#include "ActorComponents/EnemyAIComponent.h"

#include "ActorComponents/CharacterStateComponent.h"
#include "Characters/CharacterBase.h"
#include "Characters/EnemyCharacter.h"
#include "CharacterStates/CharacterStates.h"
#include "Kismet/GameplayStatics.h"
#include "BattleSystem/WaveEnemyManager.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameModes/SpinningRiot.h"
#include "Engine/World.h"
#include "EngineUtils.h"

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
	if (!IsValid(PlayerPawn)) PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	const ASpinningRiot* Demo = GetWorld()->GetAuthGameMode<ASpinningRiot>();
	if (Demo && Demo->GetDemoPhase() != EDemoPhase::Play) return;

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

	// LaunchCharacter queues the impulse until CharacterMovement's next update.
	// Don't chase or stop for a ranged attack before that update has applied it.
	if (!OwnerCharacter->GetCharacterMovement()->PendingLaunchVelocity.IsNearlyZero())
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

void UEnemyAIComponent::MoveWithSeparation(const FVector& DesiredOffset, float DeltaTime)
{
	FVector SeparationVelocity = FVector::ZeroVector;
	const FVector OwnLocation = OwnerCharacter->GetActorLocation();
	const FVector ChaseDirection = DesiredOffset.GetSafeNormal2D();
	if (SeparationDistance > 0.f && SeparationSpeed > 0.f)
	{
		// Includes placed enemies and wave spawns; dead enemies no longer occupy crowd space.
		for (TActorIterator<AEnemyCharacter> It(GetWorld()); It; ++It)
		{
			const AEnemyCharacter* OtherEnemy = *It;
			if (OtherEnemy == OwnerCharacter || OtherEnemy->IsDead()) continue;
			FVector Away = OwnLocation - OtherEnemy->GetActorLocation();
			if (FMath::Abs(Away.Z) > SeparationDistance) continue;
			Away.Z = 0.f;
			const float Distance = Away.Size();
			if (Distance >= SeparationDistance) continue;

			// Opposite directions for coincident spawns, stable across frames.
			Away = Distance > KINDA_SMALL_NUMBER ? Away / Distance
				: FVector(OwnerCharacter->GetUniqueID() < OtherEnemy->GetUniqueID() ? -1.f : 1.f, 0.f, 0.f);
			const float Weight = 1.f - Distance / SeparationDistance;
			SeparationVelocity += Away * Weight * SeparationSpeed;

			// Pure repulsion can stall a queue behind the front enemy; steer around it.
			if (FVector::DotProduct(ChaseDirection, Away) < -0.85f)
			{
				const float Side = (OwnerCharacter->GetUniqueID() % 2) == 0 ? 1.f : -1.f;
				SeparationVelocity += FVector::CrossProduct(FVector::UpVector, ChaseDirection)
					* Side * Weight * SeparationSpeed;
			}
		}
	}

	const float MaxStep = FMath::Max(MoveSpeed * DeltaTime, 0.f);
	const FVector MovementOffset = (DesiredOffset + SeparationVelocity * DeltaTime).GetClampedToMaxSize(MaxStep);
	if (!MovementOffset.IsNearlyZero())
	{
		OwnerCharacter->AddActorWorldOffset(MovementOffset, true);
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
	PlayerLocation.Z = EnemyLocation.Z;

	float DistanceToPlayer = FVector::Dist(EnemyLocation, PlayerLocation);
	if (DistanceToPlayer > DetectionRange)
	{
		return;
	}

	const float DesiredAttackRange = bUseRangedAttack ? FMath::Max(RangedAttackRange, 1.f) : FMath::Max(AttackRange, 0.f);
	FVector ChaseOffset = FVector::ZeroVector;
	if (DistanceToPlayer > DesiredAttackRange)
	{
		const float StopDistance = bUseRangedAttack
			? FMath::Clamp(RangedStopDistance, 0.f, DesiredAttackRange) : DesiredAttackRange * 0.95f;
		const float ChaseStep = FMath::Min(FMath::Max(MoveSpeed * DeltaTime, 0.f), DistanceToPlayer - StopDistance);
		ChaseOffset = (PlayerLocation - EnemyLocation).GetSafeNormal() * ChaseStep;
	}
	// Also separate while waiting for cooldown/token; CanAct above protects attacks and knockback.
	MoveWithSeparation(ChaseOffset, DeltaTime);
	DistanceToPlayer = FVector::Dist2D(OwnerCharacter->GetActorLocation(), PlayerLocation);
	FacePlayer();
	if (DistanceToPlayer > DesiredAttackRange) return;

	if (bUseRangedAttack)
	{
		OwnerCharacter->GetCharacterMovement()->StopMovementImmediately();
		if (!CanAttack()) return;
		const bool bHasRangedToken = !WaveManager || WaveManager->RequestAttackToken(OwnerCharacter);
		if (!bHasRangedToken) return;
		LastAttackTime = GetWorld()->GetTimeSeconds();
		if (OwnerCharacter->DoAttack(EAttackType::Ranged))
		{
			if (TokenReleaseDelay > 0.f)
			{
				GetWorld()->GetTimerManager().SetTimer(AttackTokenReleaseTimerHandle, this,
					&UEnemyAIComponent::ReleaseAttackToken, TokenReleaseDelay, false);
			}
			else ReleaseAttackToken();
		}
		else
		{
			ReleaseAttackToken();
			if (!bLoggedMissingAttackSetup)
			{
				bLoggedMissingAttackSetup = true;
				UE_LOG(LogTemp, Warning, TEXT("%s Ranged attack failed; assign AttackData.Ranged.Projectile."), *OwnerCharacter->GetName());
			}
		}
		return;
	}

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
}
