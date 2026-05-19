#pragma once

#include "CoreMinimal.h"
#include "AkComponent.h"
#include "Components/UPlayerLightComponent.h"
#include "GameFramework/Character.h"
#include "Interfaces/IPlayer.h"
#include "Interfaces/IDamageable.h"
#include "DataAssets/PlayerData.h"
#include "Components/UInventoryComponent.h"
#include "Components/UInteractionComponent.h"
#include "NiagaraComponent.h"
#include "APlayerCharacter.generated.h"

class ALitter;
class ACarriable;
class UHealthComponent;
class UStaminaComponent;
class UItemComponent;
class UWidgetComponent;
class UWorldProgressBar;
class UStaticMeshComponent;
class UWorldPlayerWidget;

UENUM()
enum class EInteractionUI : uint8
{
	None,
	LitterInventory,
	ContainerInventory,
	Lockpick,
	Chest,
};

UCLASS()
class THEDAWNFINDERS_API AAPlayerCharacter : public ACharacter, public IPlayerInterface, public IDamageable, public IInteractible
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void ApplyPlayerData();

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	bool IsReadyForRPCs() const;


// Components + Constructor
public:
	AAPlayerCharacter();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Config")
	UPlayerData* PlayerConfig;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UInventoryComponent* InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStaminaComponent* StaminaComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UItemComponent* ItemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UWidgetComponent* PlayerWidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UInteractionComponent* InteractionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* WeaponMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* GunMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UPlayerLightComponent* LightComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* ThrowablePreviewMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* WeaponCollisionPosRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* CarriablePosRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* ThrowStartPosRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USphereComponent* NoiseZone;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USphereComponent* LoudNoiseZone;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	UPointLightComponent* PointLight;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	USphereComponent* ProtectionZone;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	USphereComponent* FogOfWarLightOn;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lantern")
	USphereComponent* FogOfWarLightOff;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Debug")
	class UDebugComponent* DebugComponent;
	
	UPROPERTY(BlueprintReadWrite,EditAnywhere)
	UAkComponent* AkComponent;


// === CURSE ===
public :
	UFUNCTION(BlueprintCallable)
	bool IsProtectedFromCurse() const;
	

// === HEALTH INTERFACE ===
public:
	virtual void ReceiveDamage_Implementation(float quantity, AActor* Origin) override;

	
// === IPLAYER METHODS ===
public:
	virtual void AddInteractibleAtRange_Implementation(AActor* Interactible) override;

	virtual void RemoveInteractibleAtRange_Implementation(AActor* Interactible) override;

	virtual void StartCarryHeavyItem_Implementation(AActor* Interactible) override;

	virtual void EndCarryHeavyItem_Implementation(AActor* Interactible) override;

	virtual void StartMashButtonQTE_Implementation(float Quantity, float DecreasePerSecond, bool Cancellable) override;

	virtual void DoCameraShake_Implementation(float Intensity) override;

	virtual void DoDamagePostProcess_Implementation(float Duration) override;

	virtual void ShowProgress_Implementation(float CurrentValue) override;

	virtual void HideProgress_Implementation() override;

	virtual void SetEquippedMesh_Implementation(UStaticMesh* NewMesh, bool bIsRanged) override;

	virtual UInventoryComponent* GetInventoryComponent_Implementation() override;

	virtual EPlayerState GetCurrentPlayerState_Implementation() override;

	virtual void RequestStateChange_Implementation(EPlayerState NewState, bool bOverrideClient = false) override;

	virtual void SetCurrentPlayerState_Implementation(EPlayerState NewState, bool bOverrideClient = false) override;

	virtual void PlayAttackMontage_Implementation(UAnimMontage* AttackMontage, float Speed) override;

	virtual void AddProtectionZone_Implementation() override;

	virtual void RemoveProtectionZone_Implementation() override;

	virtual float GetSoundAlertness_Implementation(FName SoundTag) override;

	virtual void PlaySoundOnServer_Implementation(FName SoundTag, float Range, float WaveStrength, FVector Loc = FVector::ZeroVector, bool bLoudNoise = false) override;

	virtual UWorldPlayerWidget* GetPlayerWidget_Implementation() override;

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Others")
	void Server_AskOwnershipPermission(AActor* Target, AController* Origin);
	
	UFUNCTION(Client, Reliable)
	void Client_OpenInteractionUI (EInteractionUI UIType, AActor* Context);

	UFUNCTION(Client, Reliable)
	void Client_CloseInteractionUI(EInteractionUI UIType, AActor* Context);

	UFUNCTION(Server, Reliable)
	void Server_SetCurrentPlayerState(EPlayerState NewState, bool bOverrideClient);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetCurrentPlayerState(EPlayerState NewState, bool bOverrideClient);


// === INTERACTIBLE INTERFACE ===
public :
	void SelectInteractible_Implementation(AActor* Interactor);
	void UnselectInteractible_Implementation(AActor* Interactor);

	

// === MOVEMENT METHODS ===
public:	
	UFUNCTION(BlueprintCallable)
	void MoveCharacter(FVector2D Input);

	UFUNCTION(BlueprintCallable)
	void StartAutoMoveCharacter(FVector TargetPos, FRotator TargetRot, AActor* Target);

	UFUNCTION(BlueprintCallable)
	void AutoMoveCharacter();

	UFUNCTION(BlueprintCallable)
	void StopAutoMoveCharacter(bool bCancel);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_StopAutoMoveCharacter(bool bCancel);
	
	UFUNCTION(Server, Reliable)
	void ServerManageRun(bool Input);

	UFUNCTION(BlueprintCallable)
	void ManageRun(bool Input);

	UFUNCTION()
	void OnRep_PlayerSpeed();

	UFUNCTION(BlueprintCallable)
	void SetPlayerSpeed(float NewSpeed, bool bInstant = false);

	UFUNCTION(BlueprintCallable)
	void SetPlayerAcceleration(float NewAcceleration);

	UFUNCTION(Server, Reliable)
	void ServerSetPlayerAcceleration(float NewAcceleration);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetPlayerSpeed(float NewSpeed, bool bInstant = false);

	UFUNCTION(Server, Reliable)
	void ServerUseZiplineItem(UItemData* ZiplineItem);

	UFUNCTION(Server, Reliable)
	void Server_SendPushInput(ALitter* Obj, FVector Input);

	UFUNCTION()
	void UpdatePushingMovement(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void Server_SetPushingState(ALitter* Obj, bool bCarrying);

	UFUNCTION()
	void StopMovementForDuration(float Duration);

	UFUNCTION()
	void RestartMovement();


// === DODGE ===
public : 
	UFUNCTION(BlueprintCallable)
	void StartDodge();

	UFUNCTION(BlueprintCallable)
	void EndDodge();

	UFUNCTION(BlueprintCallable)
	void ActualiseDodge(float DeltaTime);


// === ROTATION / AUTO-LOCK ===
public :
	UFUNCTION(BlueprintCallable)
	void ActualiseRotation();

	UFUNCTION(BlueprintCallable)
	void ForceRotation(FVector Input);

	UFUNCTION(BlueprintCallable)
	void ForceRotationInstant(FRotator Rotation);

	UFUNCTION(Server, Unreliable, BlueprintCallable)
	void Server_StopForceRotation(float Progress);

	UFUNCTION(NetMulticast, Unreliable, BlueprintCallable)
	void Multicast_StopForceRotation(float Progress);

	UFUNCTION(BlueprintCallable)
	void StopForceRotation(float Progress);

	UFUNCTION(Server, Unreliable, BlueprintCallable)
	void Server_ForceRotation(FRotator Rotation, FVector Input, float Progress);

	UFUNCTION(NetMulticast, Unreliable, BlueprintCallable)
	void Multicast_ForceRotation(FRotator Rotation, FVector Input, float Progress, bool bOverrideClient);

	UFUNCTION()
	void StartAutoLock(float AutoLockStrength);

	UFUNCTION()
	void ActualiseAutoLock();

	UFUNCTION()
	void StopAutoLock();


// === DEATH METHODS ===
public:
	UFUNCTION()
	void OnRevive();
	UFUNCTION(Client,Reliable)
	void Client_ResetCamera();

	UFUNCTION(Server, Reliable)
	void Server_OnRevive();


// === MONTAGE METHODS ===
public :
	UFUNCTION(Server, Reliable)
	void ServerPlayMontage(UAnimMontage* Montage, float Speed);

	UFUNCTION(NetMulticast,Reliable)
	void MulticastPlayMontage(UAnimMontage* Montage, float Speed);

	UFUNCTION(BlueprintCallable)
	void PlayMontage(UAnimMontage* Montage, float Speed);

	UFUNCTION(Server, Reliable)
	void ServerPlayMontageLoop(UAnimMontage* Montage, float Speed);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayMontageLoop(UAnimMontage* Montage, float Speed);

	UFUNCTION(BlueprintCallable)
	void PlayMontageLoop(UAnimMontage* Montage, float Speed);

	UFUNCTION(BlueprintCallable)
	void InterruptMontage();

	UFUNCTION(Server, Reliable)
	void Server_InterruptMontage();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_InterruptMontage();

	UFUNCTION()
	void OnLoopMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
	void BP_OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
	void BP_OnMontageNotifyBegin(FName NotifyName);


public :
	FVector GetCurrentRotationInput() { return CurrentRotationInput; }
	bool GetIsForcingRotation() { return bIsForcingRotation; }
	bool GetAutoLockIsActive() { return bAutoLockIsActive; }


// === OTHERS ===
protected:
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_PlayNoise(FName SoundTag, float Range, FVector Location = FVector::ZeroVector, bool bLoudNoise = false);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void DisplayThrowPreview(FVector Direction, float Strength);

	UFUNCTION()
	void HideThrowPreview();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void PlayVibrations(bool bStrong, bool bLong);

	UFUNCTION(Server, Unreliable)
	void Server_SetMoveInputActive(bool bActive);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReachAutoMoveDestination();

	UFUNCTION(Server, Reliable)
	void Server_EndCarryHeavyItem();

	UFUNCTION(BlueprintNativeEvent)
	void GetShopItems();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "InitNames")
	void InitPlayerNames();

	

// === PUBLIC PROPERTIES ===
public :
	UPROPERTY(ReplicatedUsing = OnRep_PlayerSpeed)
	float PlayerSpeed = 400.0f;

	UPROPERTY(Replicated, BlueprintReadWrite)
	int PlayerIndex;

	UPROPERTY(BlueprintReadOnly)
	FVector2D CurrentDir;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPlayerState CurrentState;

	UPROPERTY(Replicated)
	int32 ProtectionZoneAmount;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bMoveInputActive;     // For root-motion animations triggers

	UPROPERTY()
	float TargetRotationRate = 360.f;

	UPROPERTY(BlueprintReadWrite)
	float CurrentNoise;

	UPROPERTY(Replicated)
	ALitter* CurrentPushedObject = nullptr;

	UPROPERTY(BlueprintReadWrite, Replicated)
	bool bIsCarrying = false;

	UPROPERTY(BlueprintReadWrite)
	bool bHoldAttackInput = false;

	UPROPERTY(BlueprintReadOnly)
	float UILoudness;
	

// === PROTECTED PROPERTIES ===
protected :
	UPROPERTY()
	UWorldPlayerWidget* PlayerWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* CrawlMontage;

	UPROPERTY(BlueprintReadOnly)
	FVector CurrentPlayerInput;

	UPROPERTY(BlueprintReadOnly)
	FVector PreviousPlayerInput;

	UPROPERTY(BlueprintReadWrite, Replicated)
	bool bIsAutoMoving = false;

	UPROPERTY(BlueprintReadWrite, Replicated)
	FVector AutoMoveTargetPos;

	UPROPERTY(BlueprintReadWrite, Replicated)
	FRotator AutoMoveTargetRot;

	UPROPERTY(BlueprintReadWrite, Replicated)
	AActor* AutoMoveTarget;

	UPROPERTY()
	float DodgeTimer;

	UPROPERTY()
	float LoudnessTimer;

	UPROPERTY()
	float TargetMaxSpeed = 400.f;

	UPROPERTY()
	float CurrentAutoLockStrength = 0.f;

	UPROPERTY(Replicated)
	bool bAutoLockIsActive = false;

	UPROPERTY(BlueprintReadWrite, Replicated)
	bool bIsForcingRotation;

	UPROPERTY(BlueprintReadWrite)
	float CurrentForcedRotationRatio;

	UPROPERTY(BlueprintReadWrite)
	FRotator CurrentForcedRotation;

	UPROPERTY(BlueprintReadWrite)
	FVector CurrentRotationInput;
	
	UPROPERTY(BlueprintReadWrite, Replicated)
	FVector CurrentOffsetRotationInput;

	UPROPERTY()
	AActor* CurrentAutoLockTarget;

	UPROPERTY()
	AActor* CurrentInteractible = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> InteractiblesAtRange;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float NoiseModifier = 1.0f;

	UPROPERTY(Replicated, BlueprintReadOnly)
	float NoMovementTimer;
};
