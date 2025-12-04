// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/UPlayerLightComponent.h"
#include "GameFramework/Character.h"
#include "Interfaces/IPlayer.h"
#include "Interfaces/IDamageable.h"
#include "PlayerData.h"
#include "Components/UInventoryComponent.h"
#include "Components/UInteractionComponent.h"
#include "APlayerCharacter.generated.h"

class UHealthComponent;
class UStaminaComponent;
class UItemComponent;
class UWidgetComponent;
class UWorldProgressBar;
class UStaticMeshComponent;


UCLASS()
class THEDAWNFINDERS_API AAPlayerCharacter : public ACharacter, public IPlayerInterface, public IDamageable
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void ApplyPlayerData();

	UFUNCTION()
	void OnRep_CurrentPlayerState();

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
	UWidgetComponent* ProgressBarComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UInteractionComponent* InteractionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* WeaponMeshComponent;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category="Components")
	UPlayerLightComponent* LightComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* ThrowablePreviewMeshComponent;
	

// === CURSE ===
public :
	UFUNCTION(BlueprintCallable)
	bool IsProtectedFromCurse() const;
	

// === IDAMAGEABLE METHODS ===
public:
	virtual void ReceiveDamage_Implementation(float quantity, AActor* Origin) override;

	
// === IPLAYER METHODS ===
public:
	virtual void AddInteractibleAtRange_Implementation(AActor* Interactible) override;

	virtual void RemoveInteractibleAtRange_Implementation(AActor* Interactible) override;

	virtual void DoCameraShake_Implementation(float Intensity, float duration) override;

	virtual void ShowProgress_Implementation(float CurrentValue) override;

	virtual void HideProgress_Implementation() override;

	virtual void SetEquippedMesh_Implementation(UStaticMesh* NewMesh) override;

	virtual UItemData* GetEquippedItem_Implementation() override;

	virtual EPlayerState GetCurrentPlayerState_Implementation() override;

	virtual void SetCurrentPlayerState_Implementation(EPlayerState NewState) override;

	virtual void PlayAttackMontage_Implementation(UAnimMontage* AttackMontage, float Speed) override;

	virtual void AddProtectionZone_Implementation() override;

	virtual void RemoveProtectionZone_Implementation() override;
	

// === MOVEMENT METHODS ===
public:	
	UFUNCTION(BlueprintCallable)
	void MoveCharacter(FVector2D Input);
	
	UFUNCTION(Server, Reliable, WithValidation)
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

	UFUNCTION()
	void OnTrapped();

	UFUNCTION(Server, Reliable)
	void Server_OnTrapped();

// === AUTO-LOCK ===
public :
	UFUNCTION()
	void StartAutoLock(float AutoLockStrength);

	UFUNCTION()
	void ActualiseAutoLock();

	UFUNCTION()
	void StopAutoLock();


// === DEATH METHODS ===
public:
	UFUNCTION()
	void OnDeath();

	UFUNCTION(Server, Reliable)
	void Server_OnDied();

	UFUNCTION()
	void OnRevive();

	UFUNCTION(Server, Reliable)
	void Server_OnRevive();

	UFUNCTION()
	void OnFallen();

	UFUNCTION(Server, Reliable)
	void Server_OnFallen();


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


// === THROW PREVIEW ===
protected:
	UFUNCTION()
	void DisplayThrowPreview(FVector Position, float Range);

	UFUNCTION()
	void HideThrowPreview();


// === PUBLIC PROPERTIES ===
public :
	UPROPERTY(ReplicatedUsing = OnRep_PlayerSpeed)
	float PlayerSpeed = 400.0f;

	UPROPERTY(BlueprintReadOnly)
	FVector2D CurrentDir;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentPlayerState, EditAnywhere, BlueprintReadWrite)
	EPlayerState CurrentState;

	UPROPERTY(Replicated)
	int32 ProtectionZoneAmount;

	UPROPERTY()
	float TargetRotationRate = 360.f;



// === PROTECTED PROPERTIES ===
protected :
	UPROPERTY()
	UWorldProgressBar* ProgressBarWidget;

	UPROPERTY(BlueprintReadOnly)
	FVector CurrentPlayerInput;

	UPROPERTY(BlueprintReadOnly)
	FVector PreviousPlayerInput;

	UPROPERTY()
	float DodgeTimer;

	UPROPERTY()
	float TargetMaxSpeed = 400.f;

	UPROPERTY()
	float CurrentAutoLockStrength = 0.f;

	UPROPERTY()
	bool bAutoLockIsActive = false;

	UPROPERTY()
	AActor* CurrentAutoLockTarget;

	UPROPERTY()
	AActor* CurrentInteractible = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> InteractiblesAtRange;

};
