#pragma once

#include "CoreMinimal.h"
#include "Components/UPlayerLightComponent.h"
#include "GameFramework/Character.h"
#include "Interfaces/IPlayer.h"
#include "Interfaces/IDamageable.h"
#include "DataAssets/PlayerData.h"
#include "Components/UInventoryComponent.h"
#include "Components/UInteractionComponent.h"
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
class THEDAWNFINDERS_API AAPlayerCharacter : public ACharacter, public IPlayerInterface, public IDamageable
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void ApplyPlayerData();

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	bool IsReadyForRPCs() const;

	UFUNCTION()
	void OnRep_CurrentPlayerState();


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
	UPlayerLightComponent* LightComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* ThrowablePreviewMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* WeaponCollisionPosRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* CarriablePosRef;
	

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

	virtual void SetEquippedMesh_Implementation(UStaticMesh* NewMesh) override;

	virtual UItemData* GetEquippedItem_Implementation() override;

	virtual EPlayerState GetCurrentPlayerState_Implementation() override;

	virtual void RequestStateChange_Implementation(EPlayerState NewState) override;

	virtual void SetCurrentPlayerState_Implementation(EPlayerState NewState) override;

	virtual void PlayAttackMontage_Implementation(UAnimMontage* AttackMontage, float Speed) override;

	virtual void AddProtectionZone_Implementation() override;

	virtual void RemoveProtectionZone_Implementation() override;

	virtual float GetSoundAlertness_Implementation(FName SoundTag) override;

	virtual void PlaySoundOnServer_Implementation(FName SoundTag, float Range, float WaveStrength, FVector Loc = FVector::ZeroVector) override;

	virtual UWorldPlayerWidget* GetPlayerWidget_Implementation() override;

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Others")
	void Server_AskOwnershipPermission(AActor* Target, AController* Origin);
	
	UFUNCTION(Client, Reliable)
	void Client_OpenInteractionUI (EInteractionUI UIType, AActor* Context);
	

// === MOVEMENT METHODS ===
public:	
	UFUNCTION(BlueprintCallable)
	void MoveCharacter(FVector2D Input);
	
	UFUNCTION(Server, Reliable)
	void ServerManageRun(bool Input);

	UFUNCTION(BlueprintCallable)
	void ManageRun(bool Input);

	UFUNCTION(BlueprintCallable)
	void StartDodge();

	UFUNCTION(BlueprintCallable)
	void EndDodge();

	UFUNCTION(BlueprintCallable)
	void ActualiseDodge(float DeltaTime);

	UFUNCTION()
	void OnRep_PlayerSpeed();

	UFUNCTION(BlueprintCallable)
	void SetPlayerSpeed(float NewSpeed);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetPlayerSpeed(float NewSpeed);

	UFUNCTION(Server, Reliable)
	void ServerUseZiplineItem(UItemData* ZiplineItem);

	UFUNCTION(Server, Reliable)
	void Server_SendPushInput(ALitter* Obj, FVector Input);

	UFUNCTION()
	void UpdatePushingMovement(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void Server_SetPushingState(ALitter* Obj, bool bCarrying);
	


// === ROTATION / AUTO-LOCK ===
public :
	UFUNCTION(BlueprintCallable)
	void ActualiseRotation();

	UFUNCTION(BlueprintCallable)
	void ForceRotation(FVector Input);

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

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
	void BP_OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
	void BP_OnMontageNotifyBegin(FName NotifyName);


// === OTHERS ===
protected:
	UFUNCTION(Server, Unreliable)
	void Server_PlaySound(FName SoundTag, float Range, FVector Location = FVector::ZeroVector);

	UFUNCTION()
	void DisplayThrowPreview(FVector Position, float Range);

	UFUNCTION()
	void HideThrowPreview();

	UFUNCTION(Server, Reliable)
	void Server_EndCarryHeavyItem();


// === PUBLIC PROPERTIES ===
public :
	UPROPERTY(ReplicatedUsing = OnRep_PlayerSpeed)
	float PlayerSpeed = 400.0f;

	UPROPERTY(BlueprintReadOnly)
	FVector2D CurrentDir;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentPlayerState)
	EPlayerState CurrentState;

	UPROPERTY(Replicated)
	int32 ProtectionZoneAmount;

	UPROPERTY()
	float TargetRotationRate = 360.f;

	UPROPERTY(Replicated)
	ALitter* CurrentPushedObject = nullptr;

	UPROPERTY(BlueprintReadWrite, Replicated)
	bool bIsCarrying = false;

	UPROPERTY(BlueprintReadOnly)
	float UILoudness;


// === PROTECTED PROPERTIES ===
protected :
	UPROPERTY()
	UWorldPlayerWidget* PlayerWidget;

	UPROPERTY(BlueprintReadOnly)
	FVector CurrentPlayerInput;

	UPROPERTY(BlueprintReadOnly)
	FVector PreviousPlayerInput;

	UPROPERTY()
	float DodgeTimer;

	UPROPERTY()
	float LoudnessTimer;

	UPROPERTY()
	float TargetMaxSpeed = 400.f;

	UPROPERTY()
	float CurrentAutoLockStrength = 0.f;

	UPROPERTY()
	bool bAutoLockIsActive = false;

	UPROPERTY(BlueprintReadWrite)
	bool bIsForcingRotation;

	UPROPERTY(BlueprintReadWrite)
	float CurrentForcedRotationRatio;

	UPROPERTY(BlueprintReadWrite)
	FRotator CurrentForcedRotation;

	UPROPERTY()
	AActor* CurrentAutoLockTarget;

	UPROPERTY()
	AActor* CurrentInteractible = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> InteractiblesAtRange;
};
