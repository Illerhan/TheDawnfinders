// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/UInventoryComponent.h"
#include "Components/UStaminaComponent.h"
#include "Components/UHealthComponent.h"
#include "Interfaces/IPlayer.h"
#include "Interfaces/IDamageable.h"
#include "APlayerCharacter.generated.h"


UENUM(BlueprintType)
enum class EPlayerState : uint8
{
	None UMETA(DisplayName = "None"),
	Running UMETA(DisplayName = "Running"),
	Crouching UMETA(DisplayName = "Crouching"),
	UsingEquipment UMETA(DisplayName = "Using Equipment"),
	Dodging UMETA(DisplayName = "Dodging"),
	Dead UMETA(DisplayName = "Dead")
};

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


// Interact Behavior
public :
	virtual void AddInteractibleAtRange_Implementation(AActor* Interactible) override;

	virtual void RemoveInteractibleAtRange_Implementation(AActor* Interactible) override;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPlayerState CurrentState;

	float DodgeTimer;
	
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	bool IsReadyForRPCs() const;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	void MoveCharacter(FVector2D Input);

	UFUNCTION(BlueprintCallable)
	void RotateCharacter();

	UFUNCTION(BlueprintCallable)
	void ManageRun(bool Input);

	UFUNCTION(BlueprintCallable)
	void Dodge();

	UFUNCTION(BlueprintCallable)
	void ActualiseDodge(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	void UseCurrentItem();
};
