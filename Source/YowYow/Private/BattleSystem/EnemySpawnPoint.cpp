#include "BattleSystem/EnemySpawnPoint.h"

#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"

AEnemySpawnPoint::AEnemySpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	Arrow->SetupAttachment(SceneRoot);
	Arrow->ArrowColor = FColor(255, 80, 80);
	Arrow->bHiddenInGame = true;
}
