// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/IPlayer.h"
#include "Interfaces/IDamageable.h"
#include "APlayerCharacter.generated.h"

class UInventoryComponent;
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

// Components + Constructor
public:
	AAPlayerCharacter();

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
	UStaticMeshComponent* WeaponMeshComponent;


// Interact Behavior
public :
	UFUNCTION(Server, Reliable)
	void ServerInteract(AInteractibleObjects* Interactible,AAPlayerCharacter* Player);

	UFUNCTION(Server, Reliable)
	void ServerStopInteract(ALever* Lever, AAPlayerCharacter* Player);
	
	UFUNCTION()
	void TryInteract(AInteractibleObjects* InteractibleObject,AAPlayerCharacter* Player);

	UPROPERTY()
	AInteractibleObjects* CurrentInteractible = nullptr;

	UFUNCTION(BlueprintCallable)
	void StartInteract();

	UFUNCTION(BlueprintCallable)
	void StopInteract();

	UFUNCTION(BlueprintCallable)
	AActor* GetNearestInteractible();

	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> InteractiblesAtRange;


// Player Interface
public : 
	virtual void AddInteractibleAtRange_Implementation(AActor* Interactible) override;

	virtual void RemoveInteractibleAtRange_Implementation(AActor* Interactible) override;

	virtual void ShowProgress_Implementation(float CurrentValue) override;

	virtual void HideProgress_Implementation() override;

	virtual void SetEquippedMesh_Implementation(UStaticMesh* NewMesh) override;

	virtual EPlayerState GetCurrentPlayerState_Implementation() override;

	virtual void SetCurrentPlayerState(EPlayerState NewState) override;

	virtual void PlayAttackMontage_Implementation(UAnimMontage* AttackMontage) override;


// Damageable Behavior
public:
	virtual void ReceiveDamage_Implementation(float quantity, AActor* Origin) override;
	

// Movement + State
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(BlueprintReadOnly)
	FVector2D CurrentDir;

	UPROPERTY(BlueprintReadOnly)
	FVector CurrentPlayerInput;

	UPROPERTY(BlueprintReadOnly)
	FVector PreviousPlayerInput;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentPlayerState,EditAnywhere, BlueprintReadWrite)
	EPlayerState CurrentState;

	UFUNCTION()
	void OnRep_CurrentPlayerState();	
	
	float DodgeTimer;
	
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	bool IsReadyForRPCs() const;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	void MoveCharacter(FVector2D Input);

	//UFUNCTION(BlueprintCallable)
	//void RotateCharacter();

	
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

	UFUNCTION(BlueprintCallable)
	void UseCurrentItem();

	UFUNCTION(Server, Reliable)
	void ServerPlayMontage(UAnimMontage* Montage);

	UFUNCTION(NetMulticast,Reliable)
	void MulticastPlayMontage(UAnimMontage* Montage);

	UFUNCTION(BlueprintCallable)
	void PlayMontage(UAnimMontage* Montage);

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(BlueprintImplementableEvent, Category="Animation")
	void BP_OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(Server, Reliable)
	void ServerUseZiplineItem(UItemData* ZiplineItem);


// Private References
private :
	UPROPERTY()
	UWorldProgressBar* ProgressBarWidget;
};
