// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase.h"
#include "Types/AttackTypes.h"
#include "Types/TrickTypes.h"
#include "EriCharacter.generated.h"

struct FInputActionValue;
class UInputAction;
class UInputMappingContext;
class UEnhancedInputLocalPlayerSubsystem;
class UEnhancedInputComponent;
class UCameraComponent;
class USpringArmComponent;
class UHomingAttackComponent;
class UTrickGaugeComponent;
class UComboComponent;
class UStaticMeshComponent;
class UStaticMesh;
class USceneComponent;
class UAttackComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class UPaperZDAnimSequence;
class AAttackHitbox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrickQTEStarted, const TArray<ETrickDirection>&, Sequence);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrickQTEIndexChanged, int32, CompletedInputs);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrickQTEEnded, bool, bSuccess);

/**
 * Player character Eri.
 * Yoyo components are created in C++; mesh assets assigned in BP.
 * Rest pose = component transform from the BP/viewport (never overwritten in BeginPlay).
 */
UCLASS()
class YOWYOW_API AEriCharacter : public ACharacterBase
{
	GENERATED_BODY()

public:
	AEriCharacter();

	/** Loads BP_EriCharacter (and its Niagara/mesh refs) so they stay resident across the title → game OpenLevel. */
	static void PreloadCharacterAssets();

	/** True while a yoyo is mid thrust/orbit/return (blocks / buffers next attack). */
	UFUNCTION(BlueprintPure, Category = "YoYo")
	bool IsYoYoPresentationActive() const { return PresentationMode != EYoYoPresentationMode::None; }

	UFUNCTION(BlueprintPure, Category = "YoYo|Animation")
	EYoYoHand GetAttackYoYoHand() const { return AttackYoYoHand; }

	UFUNCTION(BlueprintPure, Category = "YoYo|Animation")
	EAttackType GetActiveAttackType() const;

	UFUNCTION(BlueprintPure, Category = "Homing")
	bool IsHomingCameraLocked() const { return bHomingCameraLocked; }

	UFUNCTION(BlueprintPure, Category = "Trick")
	bool IsTrickInputLocked() const { return bTrickQTEActive || bDNAExecuting; }

	UFUNCTION(BlueprintPure, Category = "Trick")
	bool IsTrickQTEActive() const { return bTrickQTEActive; }

	UFUNCTION(BlueprintPure, Category = "Trick")
	TArray<ETrickDirection> GetTrickQTESequence() const { return TrickQTESequence; }

	UFUNCTION(BlueprintPure, Category = "Trick")
	int32 GetTrickQTEIndex() const { return TrickQTEIndex; }

	void CancelDemoActions();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;
	virtual void Landed(const FHitResult& Hit) override;

	UPROPERTY(EditAnywhere, Category = "Input Actions|Movement")
	UInputAction* MovementAction = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input Actions|Movement")
	UInputAction* JumpAction = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input Actions|Movement")
	UInputAction* LookAction = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input Actions|Combat")
	UInputAction* AttackAction = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input Actions|Combat")
	UInputAction* AreaAttackAction = nullptr;

	/** Deprecated: air light attack starts homing. Kept for serialized BP assignments. */
	UPROPERTY(EditAnywhere, Category = "Input Actions|Combat")
	UInputAction* HomingAction = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input Actions|Trick")
	UInputAction* TrickModeAction = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input Actions|Trick")
	UInputAction* TrickInputAction = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="VFX", meta=(AllowPrivateAccess="true"))
	UNiagaraComponent* YoYoRightVFX = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="VFX", meta=(AllowPrivateAccess="true"))
	UNiagaraComponent* YoYoLeftVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="VFX|YoYo")
	UNiagaraSystem* NormalAttackVFXSystem = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="VFX|YoYo")
	UNiagaraSystem* AreaAttackVFXSystem = nullptr;

	UPROPERTY()
	UNiagaraSystem* CurrentYoYoAttackVFX = nullptr;

	void StartYoYoAttackVFX();
	void StopYoYoAttackVFX();
	
	/** Looping aura during QTE and both DNA phases. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Trick")
	UNiagaraComponent* TrickAuraVFX = nullptr;

	/** Niagara system assigned in BP for the Trick Mode aura. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Trick")
	UNiagaraSystem* TrickAuraVFXSystem = nullptr;
	
	// Just to debug the VFX
	UPROPERTY(EditAnywhere, Category = "Input Actions|Debug")
	UInputAction* TrickAuraDebugAction = nullptr;

	/** Unbound legacy preview toggle. Gameplay activates the aura directly. */
	void ToggleTrickAuraVFX();

	void Move(const FInputActionValue& Value);
	void JumpPressed();
	void JumpReleased();
	void Look(const FInputActionValue& Value);
	void TryAttack();
	void TryAreaAttack();
	void TryHomingAttack();
	void EnterTrickMode();
	void ExitTrickMode();
	void TryTrickInput(const FInputActionValue& Value);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UTrickGaugeComponent* TrickGaugeComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UComboComponent* ComboComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UHomingAttackComponent* HomingAttackComponent = nullptr;

	/** Consumed from UHomingAttackComponent::OnHomingAttackFinished. */
	UFUNCTION()
	void HandleHomingAttackFinished(const bool bSuccess);

	/** Consumed from UAttackComponent::OnAttackStarted. */
	UFUNCTION()
	void HandleAttackStarted(EAttackType AttackType, FAttackData StartedAttackData);

	/** Consumed from UAttackComponent::OnAttackFinished (hit window done → start return if needed). */
	UFUNCTION()
	void HandleAttackFinished(EAttackType AttackType, bool bCompleted);

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* CameraBoom = nullptr;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera = nullptr;

	/** Set Static Mesh + relative transform in the BP viewport (position is NOT overwritten in code). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "YoYo")
	UStaticMeshComponent* YoYoRight = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "YoYo")
	UStaticMeshComponent* YoYoLeft = nullptr;

	/** Optional: apply this mesh to both hands in BeginPlay if set. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YoYo|Mesh")
	UStaticMesh* YoYoMeshAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YoYo|Mesh")
	UStaticMesh* YoYoLeftMeshAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YoYo", meta = (ClampMin = "0.1"))
	float YoYoReturnSpeedMultiplier = 1.5f;

	/** Paper2D socket on each sprite frame. Idle/run attach here; attack detaches. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YoYo|Sockets")
	FName YoYoRightSocketName = TEXT("Hand_R");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YoYo|Sockets")
	FName YoYoLeftSocketName = TEXT("Hand_L");

	/** PaperZD sequences: same mapping as attack YoYoHand (Normal 0/1/2). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YoYo|Animation")
	TObjectPtr<UPaperZDAnimSequence> AttackAnimSequenceRight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YoYo|Animation")
	TObjectPtr<UPaperZDAnimSequence> AttackAnimSequenceLeft;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YoYo|Animation")
	TObjectPtr<UPaperZDAnimSequence> AttackAnimSequenceBoth;

	/** Optional; if unset, area uses AttackAnimSequenceBoth. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YoYo|Animation")
	TObjectPtr<UPaperZDAnimSequence> AreaAttackAnimSequence;

	/** How fast control yaw catches up to flight direction while homing (deg/s). 0 = snap. Fast but readable. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Homing|Camera", meta = (ClampMin = "0.0"))
	float HomingCameraYawInterpSpeed = 540.f;

	/** If true, pitch look is also blocked during homing lock (yaw always blocked). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Homing|Camera")
	bool bHomingCameraLockPitch = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trick|DNA|Animation")
	TObjectPtr<UPaperZDAnimSequence> DNAPhaseOneSequence;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trick|DNA|Animation")
	TObjectPtr<UPaperZDAnimSequence> DNAPhaseTwoSequence;

public:
	UPROPERTY(BlueprintAssignable, Category = "Trick|QTE")
	FOnTrickQTEStarted OnTrickQTEStarted;

	UPROPERTY(BlueprintAssignable, Category = "Trick|QTE")
	FOnTrickQTEIndexChanged OnTrickQTEIndexChanged;

	UPROPERTY(BlueprintAssignable, Category = "Trick|QTE")
	FOnTrickQTEEnded OnTrickQTEEnded;

private:
	void SetHomingCameraLocked(bool bLocked);
	void UpdateHomingCameraLock(float DeltaTime);
	FVector GetHomingCameraFacingDirection() const;
	enum class EYoYoPresentationMode : uint8
	{
		None,
		Thrust,
		Orbit,
		Homing,
		DNASurround,
		DNABurst
	};

	struct FYoYoRuntime
	{
		TWeakObjectPtr<USceneComponent> Component;
		FName SocketName = NAME_None;
		FVector RestRelative = FVector::ZeroVector;
		FRotator RestRelativeRotation = FRotator::ZeroRotator;
		FVector OutboundWorld = FVector::ZeroVector;
		/** World position at the start of the current thrust/return lerp. */
		FVector PathStartWorld = FVector::ZeroVector;
		/** +1 left crescent, -1 right crescent (Orbit presentation). */
		float OrbitSideSign = -1.f;
		bool bActive = false;
	};

	void ApplyYoYoMeshAssets();
	void CacheYoYoRests();
	void AttachYoYosToHandSockets();
	void AttachYoYoToHandSocket(
		UStaticMeshComponent* YoYo,
		FName SocketName,
		const FVector& FallbackRelative,
		const FRotator& RestRotation
	);
	void DetachYoYoForFlight(USceneComponent* YoYo);
	void BeginYoYoPresentation(const FAttackData& InAttackData);
	void BeginHomingYoyoCharge();
	void UpdateYoYoPresentation(float DeltaTime);
	void UpdateHomingYoyos(float DeltaTime);
	void StartYoYoReturn();
	void FinishYoYoPresentation();
	void GatherHands(EYoYoHand Hand, TArray<FYoYoRuntime*>& OutHands);
	FVector GetRestWorldLocation(const FYoYoRuntime& Hand) const;
	bool AreActiveYoYosAtTarget(bool bReturning) const;
	void ClearPresentationAttackType();

	FYoYoRuntime RightHand;
	FYoYoRuntime LeftHand;

	EYoYoPresentationMode PresentationMode = EYoYoPresentationMode::None;
	FVector CachedAttackForward = FVector::ForwardVector;
	float YoYoCurrentSpeed = 600.f;
	float OrbitRadius = 100.f;
	/** Degrees traveled this crescent pass (capped at 180 — back to front once). */
	float OrbitTravelDegrees = 0.f;
	/** Shared 0→1 lerp so both yoyos arrive at the triangle apex together. */
	float ThrustElapsed = 0.f;
	float ThrustDuration = 0.f;
	bool bYoYoReturning = false;
	EYoYoHand AttackYoYoHand = EYoYoHand::Right;
	EAttackType PresentationAttackType = EAttackType::None;

	/** Camera locked behind Eri during homing dash (back sprite only). */
	bool bHomingCameraLocked = false;

	void FailTrickQTE();
	void SetupTrickKeyboardInput(UEnhancedInputComponent* EnhancedInput);
	void SetTrickKeyboardContextEnabled(bool bEnabled);
	void SubmitTrickDirection(ETrickDirection Cardinal);
	void ResetTrickCardinal();
	void ExitTrickModeInternal();
	void BeginDNAPresentation(const FAttackData& InAttackData);
	void BeginDNABurst();
	void UpdateDNAPresentation(float DeltaTime);
	void FinishDNAPresentation();
	void ClearDNAHitboxes();
	void PlayDNASequence(UPaperZDAnimSequence* Sequence, float PhaseDuration);
	AAttackHitbox* SpawnDNAHitbox(const FAttackData& InAttackData, float AngleDegrees);

	TArray<ETrickDirection> TrickQTESequence;
	/** Per-key actions consume WASD before the shared stick action can aggregate them. */
	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> TrickKeyboardContext;
	TWeakObjectPtr<UEnhancedInputLocalPlayerSubsystem> TrickKeyboardSubsystem;

	int32 TrickQTEIndex = 0;
	ETrickDirection LastQTECardinal = ETrickDirection::None;
	bool bTrickQTEActive = false;
	bool bDNAExecuting = false;
	float DNAPhaseElapsed = 0.f;
	float DNAOrbitDuration = 1.2f;
	float DNABurstDuration = 0.75f;
	float DNAFlightRange = 600.f;
	float DNAFlightSpeed = 800.f;
	FAttackData DNAResolvedAttack;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AAttackHitbox>> DNAHitboxes;
};
