#include "Attacks/EnemyProjectile.h"

#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameModes/SpinningRiot.h"
#include "Kismet/GameplayStatics.h"

AEnemyProjectile::AEnemyProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	ProjectileCollision = CreateDefaultSubobject<USphereComponent>(TEXT("ProjectileCollision"));
	SetRootComponent(ProjectileCollision);
	ProjectileCollision->InitSphereRadius(12.f);
	ProjectileCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ProjectileCollision->SetCollisionObjectType(ECC_WorldDynamic);
	ProjectileCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	ProjectileCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ProjectileCollision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	ProjectileCollision->SetGenerateOverlapEvents(true);
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetUpdatedComponent(ProjectileCollision);
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bForceSubStepping = true;
	ProjectileMovement->InitialSpeed = 800.f;
	ProjectileMovement->MaxSpeed = 800.f;
}

void AEnemyProjectile::BeginPlay()
{
	Super::BeginPlay();
	ProjectileCollision->IgnoreActorWhenMoving(GetOwner(), true);
	ProjectileCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemyProjectile::HandleProjectileOverlap);
	ProjectileMovement->OnProjectileStop.AddDynamic(this, &AEnemyProjectile::HandleProjectileImpact);
	ProjectileMovement->InitialSpeed = FMath::Max(ProjectileSpeed, 1.f);
	ProjectileMovement->MaxSpeed = ProjectileMovement->InitialSpeed;
	ProjectileMovement->Velocity = GetActorForwardVector() * ProjectileMovement->InitialSpeed;
	SetLifeSpan(FMath::Max(ProjectileLifetime, 0.1f));
}

void AEnemyProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	const ASpinningRiot* Demo = GetWorld()->GetAuthGameMode<ASpinningRiot>();
	if (Demo && Demo->GetDemoPhase() != EDemoPhase::Play) Destroy();
}

void AEnemyProjectile::HandleProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const APawn* HitPawn = Cast<APawn>(OtherActor);
	if (bProjectileConsumed || !HitPawn || !HitPawn->IsPlayerControlled() || OtherActor == GetOwner()) return;
	bProjectileConsumed = true;
	UGameplayStatics::ApplyDamage(OtherActor, ProjectileDamage, GetInstigatorController(), this, nullptr);
	Destroy();
}

void AEnemyProjectile::HandleProjectileImpact(const FHitResult& ImpactResult)
{
	Destroy();
}
