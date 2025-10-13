// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/UInventoryComponent.h"
#include "Interfaces/IInteractible.h"
#include "Interfaces/IPlayer.h"
#include "APlayerCharacter.generated.h"

UENUM(BlueprintType)
enum class EPlayerState : uint8
{
	None UMETA(DisplayName = "None"),
	Running UMETA(DisplayName = "Running"),
	Crouching UMETA(DisplayName = "Crouching"),
	UsingEquipment UMETA(DisplayName = "Using Equipment"),
	Dead UMETA(DisplayName = "Dead")
};

UCLASS()
class THEDAWNFINDERS_API AAPlayerCharacter : public ACharacter, public IPlayerInterface
{
	GENERATED_BODY()

public:
	AAPlayerCharacter();

	virtual void AddInteractibleAtRange_Implementation(AActor* Interactible) override;
	virtual void RemoveInteractibleAtRange_Implementation(AActor* Interactible) override;

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(BlueprintReadOnly)
	FVector2D CurrentDir;

	UPROPERTY(BlueprintReadOnly)
	FVector CurrentPlayerInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPlayerState CurrentState;

	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> InteractiblesAtRange;

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
	AActor* GetNearestInteractible();


};
