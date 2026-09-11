#include "Characters/EriCharacter.h"

#include "ActorComponents/AttackComponent.h"
#include "ActorComponents/HomingAttackComponent.h"
#include "Attacks/AttackHitbox.h"
#include "Components/StaticMeshComponent.h"
#include "DataAssets/CharacterAttackData.h"
#include "Engine/World.h"
#include "NiagaraComponent.h"
#include "PaperZDAnimInstance.h"
#include "AnimSequences/PaperZDAnimSequence.h"

AAttackHitbox* AEriCharacter::SpawnDNAHitbox(const FAttackData& InAttackData, float AngleDegrees)
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AAttackHitbox* DNAHitbox = GetWorld()->SpawnActor<AAttackHitbox>(
		AAttackHitbox::StaticClass(), FTransform(FRotator::ZeroRotator, GetActorLocation()), SpawnParameters);
	if (DNAHitbox)
	{
		DNAHitbox->InitializeDNA(this, InAttackData, AngleDegrees);
		DNAHitboxes.Add(DNAHitbox);
	}
	return DNAHitbox;
}

void AEriCharacter::PlayDNASequence(UPaperZDAnimSequence* Sequence, float PhaseDuration)
{
	if (UPaperZDAnimInstance* DNAAnimInstance = GetAnimInstance())
	{
		DNAAnimInstance->StopAnimationOverrideByGroup(TEXT("DefaultGroup"));
		if (Sequence)
		{
			const float SequenceDuration = Sequence->GetTotalDuration();
			const float PhasePlayRate = SequenceDuration > 0.f ? SequenceDuration / FMath::Max(PhaseDuration, 0.01f) : 1.f;
			DNAAnimInstance->PlayAnimationOverride(Sequence, TEXT("DefaultSlot"), PhasePlayRate);
		}
	}
}

void AEriCharacter::BeginDNAPresentation(const FAttackData& InAttackData)
{
	ClearDNAHitboxes();
	bDNAExecuting = true;
	bYoYoReturning = false;
	PresentationMode = EYoYoPresentationMode::DNASurround;
	PresentationAttackType = EAttackType::DNA;
	AttackYoYoHand = EYoYoHand::Both;
	DNAPhaseElapsed = 0.f;
	DNAResolvedAttack = InAttackData;
	DNAResolvedAttack.Motion = EAttackMotion::OrbitOwner;
	DNAResolvedAttack.Speed = FMath::Max(InAttackData.Speed, 1.f);
	DNAResolvedAttack.Range = FMath::Max(InAttackData.Range, 1.f);
	DNAOrbitDuration = AttackData ? FMath::Max(AttackData->DNAPhase1Duration, 0.1f) : 1.2f;
	DNAFlightRange = AttackData ? FMath::Max(AttackData->DNABurstRange, 1.f) : 600.f;
	DNAFlightSpeed = AttackData ? FMath::Max(AttackData->DNABurstSpeed, 1.f) : 800.f;
	DNABurstDuration = DNAFlightRange / DNAFlightSpeed;
	RightHand.bActive = true;
	LeftHand.bActive = true;
	DetachYoYoForFlight(YoYoRight);
	DetachYoYoForFlight(YoYoLeft);
	CurrentYoYoAttackVFX = AreaAttackVFXSystem;
	StartYoYoAttackVFX();
	const float InitialAngle = GetActorRotation().Yaw;
	AAttackHitbox* RightOrbit = SpawnDNAHitbox(DNAResolvedAttack, InitialAngle);
	AAttackHitbox* LeftOrbit = SpawnDNAHitbox(DNAResolvedAttack, InitialAngle + 180.f);
	if (!RightOrbit || !LeftOrbit)
	{
		FinishDNAPresentation();
		return;
	}
	if (YoYoRight) YoYoRight->SetWorldLocation(RightOrbit->GetActorLocation());
	if (YoYoLeft) YoYoLeft->SetWorldLocation(LeftOrbit->GetActorLocation());
	PlayDNASequence(DNAPhaseOneSequence, DNAOrbitDuration);
}

void AEriCharacter::BeginDNABurst()
{
	ClearDNAHitboxes();
	StopYoYoAttackVFX();
	if (YoYoRight) YoYoRight->SetHiddenInGame(true, true);
	if (YoYoLeft) YoYoLeft->SetHiddenInGame(true, true);
	PresentationMode = EYoYoPresentationMode::DNABurst;
	DNAPhaseElapsed = 0.f;
	FAttackData BurstAttack = DNAResolvedAttack;
	BurstAttack.Motion = EAttackMotion::RadialBurst;
	BurstAttack.Range = DNAFlightRange;
	BurstAttack.Speed = DNAFlightSpeed;
	// Demo v1 always emits six rays, even if an asset contains an invalid count.
	constexpr int32 BurstCount = 6;
	for (int32 RayIndex = 0; RayIndex < BurstCount; ++RayIndex)
	{
		AAttackHitbox* BurstHitbox = SpawnDNAHitbox(BurstAttack, GetActorRotation().Yaw + RayIndex * 60.f);
		if (!BurstHitbox)
		{
			continue;
		}
		UStaticMeshComponent* TemplateYoYo = RayIndex % 2 == 0 ? YoYoRight : YoYoLeft;
		UStaticMeshComponent* BurstVisual = NewObject<UStaticMeshComponent>(BurstHitbox, TEXT("DNABurstVisual"));
		BurstVisual->SetupAttachment(BurstHitbox->GetRootComponent());
		BurstVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (TemplateYoYo)
		{
			BurstVisual->SetStaticMesh(TemplateYoYo->GetStaticMesh());
			BurstVisual->SetRelativeScale3D(TemplateYoYo->GetComponentScale());
			BurstVisual->SetRelativeRotation(TemplateYoYo->GetComponentRotation());
			for (int32 MaterialIndex = 0; MaterialIndex < TemplateYoYo->GetNumMaterials(); ++MaterialIndex)
			{
				BurstVisual->SetMaterial(MaterialIndex, TemplateYoYo->GetMaterial(MaterialIndex));
			}
		}
		BurstHitbox->AddInstanceComponent(BurstVisual);
		BurstVisual->RegisterComponent();
		if (AreaAttackVFXSystem)
		{
			UNiagaraComponent* BurstTrail = NewObject<UNiagaraComponent>(BurstHitbox, TEXT("DNABurstTrail"));
			BurstTrail->SetupAttachment(BurstVisual);
			BurstTrail->SetAutoActivate(false);
			BurstTrail->SetAsset(AreaAttackVFXSystem);
			BurstHitbox->AddInstanceComponent(BurstTrail);
			BurstTrail->RegisterComponent();
			BurstTrail->Activate(true);
		}
	}
	PlayDNASequence(DNAPhaseTwoSequence, DNABurstDuration);
}

void AEriCharacter::UpdateDNAPresentation(float DeltaTime)
{
	const bool bOrbitPhase = PresentationMode == EYoYoPresentationMode::DNASurround;
	const float PhaseDuration = bOrbitPhase ? DNAOrbitDuration : DNABurstDuration;
	DNAPhaseElapsed = FMath::Min(DNAPhaseElapsed + FMath::Max(DeltaTime, 0.f), PhaseDuration);
	// Damage delegates may end the demo and clear the original array.
	const TArray<TObjectPtr<AAttackHitbox>> PhaseHitboxes = DNAHitboxes;
	for (int32 HitboxIndex = 0; HitboxIndex < PhaseHitboxes.Num(); ++HitboxIndex)
	{
		AAttackHitbox* DNAHitbox = PhaseHitboxes[HitboxIndex];
		if (!bDNAExecuting || !IsValid(DNAHitbox))
		{
			return;
		}
		DNAHitbox->UpdateDNAMotion(DNAPhaseElapsed);
		if (!bDNAExecuting || !IsValid(DNAHitbox)) return;
		if (bOrbitPhase)
		{
			UStaticMeshComponent* OrbitVisual = HitboxIndex == 0 ? YoYoRight : YoYoLeft;
			if (OrbitVisual) OrbitVisual->SetWorldLocation(DNAHitbox->GetActorLocation());
		}
	}
	if (DNAPhaseElapsed >= PhaseDuration)
	{
		if (bOrbitPhase) BeginDNABurst();
		else FinishDNAPresentation();
	}
}

void AEriCharacter::ClearDNAHitboxes()
{
	for (AAttackHitbox* DNAHitbox : DNAHitboxes)
	{
		if (IsValid(DNAHitbox)) DNAHitbox->Destroy();
	}
	DNAHitboxes.Reset();
}

void AEriCharacter::FinishDNAPresentation()
{
	ClearDNAHitboxes();
	bDNAExecuting = false;
	if (UPaperZDAnimInstance* DNAAnimInstance = GetAnimInstance())
	{
		DNAAnimInstance->StopAnimationOverrideByGroup(TEXT("DefaultGroup"));
	}
	if (YoYoRight) YoYoRight->SetHiddenInGame(false, true);
	if (YoYoLeft) YoYoLeft->SetHiddenInGame(false, true);
	FinishYoYoPresentation();
	ExitTrickModeInternal();
}

void AEriCharacter::CancelDemoActions()
{
	if (bTrickQTEActive) FailTrickQTE();
	if (HomingAttackComponent) HomingAttackComponent->CancelHomingAttack();
	if (AttackComponent) AttackComponent->CancelActiveAttack();
	if (bDNAExecuting) FinishDNAPresentation();
	else if (IsYoYoPresentationActive()) FinishYoYoPresentation();
	SetHomingCameraLocked(false);
}
