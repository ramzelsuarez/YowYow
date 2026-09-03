#include "ActorComponents/ComboComponent.h"

#include "ActorComponents/HealthComponent.h"
#include "Engine/Engine.h"
#include "Interfaces/Comboable.h"
#include "UObject/Class.h"

UComboComponent::UComboComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	BuildDefaultTiers();
}

void UComboComponent::BuildDefaultTiers()
{
	Tiers = {
		FComboTierDef(EComboTier::D, 1, FLinearColor(0.75f, 0.75f, 0.8f)),
		FComboTierDef(EComboTier::C, 40, FLinearColor(0.3f, 0.85f, 1.f)),
		FComboTierDef(EComboTier::B, 90, FLinearColor(0.25f, 0.45f, 1.f)),
		FComboTierDef(EComboTier::A, 150, FLinearColor(0.7f, 0.3f, 1.f)),
		FComboTierDef(EComboTier::S, 230, FLinearColor(1.f, 0.85f, 0.2f)),
		FComboTierDef(EComboTier::SS, 320, FLinearColor(1.f, 0.5f, 0.1f)),
		FComboTierDef(EComboTier::SSS, 420, FLinearColor(1.f, 0.15f, 0.15f)),
	};
}

void UComboComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Tiers.IsEmpty())
	{
		BuildDefaultTiers();
	}
	SortTiers();

	if (AActor* Owner = GetOwner())
	{
		if (UHealthComponent* Health = Owner->FindComponentByClass<UHealthComponent>())
		{
			Health->OnHealthChanged.AddDynamic(this, &UComboComponent::HandleOwnerHealthChanged);
		}
	}
}

void UComboComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AActor* Owner = GetOwner())
	{
		if (UHealthComponent* Health = Owner->FindComponentByClass<UHealthComponent>())
		{
			Health->OnHealthChanged.RemoveDynamic(this, &UComboComponent::HandleOwnerHealthChanged);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UComboComponent::NotifyHit(AActor* InstigatorPawn, AActor* HitActor)
{
	if (!IsValid(InstigatorPawn) || !IsValid(HitActor) || !HitActor->Implements<UComboable>())
	{
		return;
	}

	// CanGrantCombo is snapshotted by the caller *before* ApplyDamage so killing blows still count.
	if (UComboComponent* Combo = InstigatorPawn->FindComponentByClass<UComboComponent>())
	{
		Combo->RegisterHit(Combo->GetHitPoints());
	}
}

void UComboComponent::RegisterHit(int32 Points)
{
	if (Points <= 0 || IsOwnerDead())
	{
		return;
	}

	CurrentPoints += static_cast<float>(Points);
	++HitCount;
	TimeSinceLastHit = 0.f;
	SetComponentTickEnabled(true);
	RecalculateTier();
	BroadcastChanged();
	DrawDebugCombo();
}

void UComboComponent::DropTier()
{
	if (CurrentTier == EComboTier::None || CurrentTier == EComboTier::D)
	{
		BreakCombo();
		return;
	}

	const FComboTierDef* Previous = nullptr;
	for (const FComboTierDef& Def : Tiers)
	{
		if (Def.Tier == CurrentTier)
		{
			break;
		}
		Previous = &Def;
	}

	if (!Previous)
	{
		BreakCombo();
		return;
	}

	CurrentPoints = static_cast<float>(Previous->MinPoints);
	TimeSinceLastHit = 0.f;
	RecalculateTier();
	BroadcastChanged();
	DrawDebugCombo();
}

void UComboComponent::BreakCombo()
{
	if (CurrentTier == EComboTier::None && CurrentPoints <= 0.f && HitCount == 0)
	{
		SetComponentTickEnabled(false);
		return;
	}

	CurrentPoints = 0.f;
	HitCount = 0;
	TimeSinceLastHit = 0.f;
	const EComboTier OldTier = CurrentTier;
	CurrentTier = EComboTier::None;
	SetComponentTickEnabled(false);

	if (OldTier != EComboTier::None)
	{
		OnComboTierChanged.Broadcast(OldTier, EComboTier::None);
	}
	BroadcastChanged();
	OnComboBroken.Broadcast();
}

FText UComboComponent::GetTierDisplayName() const
{
	if (const UEnum* Enum = StaticEnum<EComboTier>())
	{
		return Enum->GetDisplayNameTextByValue(static_cast<int64>(CurrentTier));
	}
	return FText::GetEmpty();
}

FLinearColor UComboComponent::GetTierColor() const
{
	if (const FComboTierDef* Def = FindTierDef(CurrentTier))
	{
		return Def->Color;
	}
	return FLinearColor::White;
}

float UComboComponent::GetProgressToNextTier() const
{
	if (CurrentTier == EComboTier::None || Tiers.IsEmpty())
	{
		return 0.f;
	}

	int32 CurrentMin = 0;
	int32 NextMin = INDEX_NONE;
	bool bFoundCurrent = false;
	for (const FComboTierDef& Def : Tiers)
	{
		if (!bFoundCurrent)
		{
			if (Def.Tier == CurrentTier)
			{
				CurrentMin = Def.MinPoints;
				bFoundCurrent = true;
			}
			continue;
		}

		NextMin = Def.MinPoints;
		break;
	}

	if (!bFoundCurrent || NextMin == INDEX_NONE)
	{
		return 1.f;
	}

	const int32 Span = NextMin - CurrentMin;
	if (Span <= 0)
	{
		return 1.f;
	}

	return FMath::Clamp((CurrentPoints - static_cast<float>(CurrentMin)) / static_cast<float>(Span), 0.f, 1.f);
}

void UComboComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentTier == EComboTier::None)
	{
		SetComponentTickEnabled(false);
		return;
	}

	TimeSinceLastHit += DeltaTime;
	if (TimeSinceLastHit >= DecayDelay)
	{
		CurrentPoints -= DecayPerSecond * DeltaTime;
		if (CurrentPoints <= 0.f)
		{
			BreakCombo();
			return;
		}

		RecalculateTier();
		BroadcastChanged();
	}

	DrawDebugCombo();
}

void UComboComponent::HandleOwnerHealthChanged(
	UHealthComponent* HealthComponent,
	int32 InCurrentHealth,
	int32 MaxHealth,
	float DeltaHealth
)
{
	if (DeltaHealth < 0.f)
	{
		DropTier();
	}
}

void UComboComponent::SortTiers()
{
	Tiers.Sort([](const FComboTierDef& A, const FComboTierDef& B)
	{
		return A.MinPoints < B.MinPoints;
	});
}

void UComboComponent::RecalculateTier()
{
	const EComboTier NewTier = ComputeTierForPoints(CurrentPoints);
	if (NewTier == CurrentTier)
	{
		return;
	}

	const EComboTier OldTier = CurrentTier;
	CurrentTier = NewTier;
	OnComboTierChanged.Broadcast(OldTier, NewTier);
}

void UComboComponent::BroadcastChanged() const
{
	OnComboChanged.Broadcast(GetCurrentPoints(), HitCount, CurrentTier);
}

void UComboComponent::DrawDebugCombo() const
{
	if (!bDebugDrawCombo || !GEngine || CurrentTier == EComboTier::None)
	{
		return;
	}

	GEngine->AddOnScreenDebugMessage(
		0xC0B0,
		1.f,
		FColor::Yellow,
		FString::Printf(
			TEXT("COMBO %s  %d HITS  %d PTS  %.0f%%"),
			*GetTierDisplayName().ToString(),
			HitCount,
			GetCurrentPoints(),
			GetProgressToNextTier() * 100.f
		)
	);
}

bool UComboComponent::IsOwnerDead() const
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return true;
	}

	const UHealthComponent* Health = Owner->FindComponentByClass<UHealthComponent>();
	return Health && Health->IsDead();
}

const FComboTierDef* UComboComponent::FindTierDef(EComboTier Tier) const
{
	return Tiers.FindByPredicate([Tier](const FComboTierDef& Def)
	{
		return Def.Tier == Tier;
	});
}

EComboTier UComboComponent::ComputeTierForPoints(float Points) const
{
	if (Points <= 0.f)
	{
		return EComboTier::None;
	}

	EComboTier Best = EComboTier::None;
	for (const FComboTierDef& Def : Tiers)
	{
		if (Points >= static_cast<float>(Def.MinPoints))
		{
			Best = Def.Tier;
		}
		else
		{
			break;
		}
	}
	return Best;
}
