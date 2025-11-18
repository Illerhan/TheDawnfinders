// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UInteractionComponent.generated.h"

class AInteractibleObjects;
class AAPlayerCharacter;

UCLASS(ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent))
class THEDAWNFINDERS_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();


// Interactibles at range management
public:
	UFUNCTION()
	void AddInteractible(AActor* Interactible);

	UFUNCTION()
	void RemoveInteractible(AActor* Interactible);

	UFUNCTION(BlueprintCallable)
	AActor* GetNearestInteractible();

	virtual void BeginPlay() override;


// Do & Stop Interaction
public:
	UFUNCTION()
	void TryInteractAlly(AAPlayerCharacter* Ally, AAPlayerCharacter* Player);

	UFUNCTION(BlueprintCallable)
	void StartInteract();

	UFUNCTION()
	void TryInteract(AInteractibleObjects* InteractibleObject, AAPlayerCharacter* Player);

	UFUNCTION(Server, Reliable)
	void ServerInteract(AInteractibleObjects* Interactible, AAPlayerCharacter* Player);

	UFUNCTION(BlueprintCallable)
	void StopInteract();

	UFUNCTION(Server, Reliable)
	void ServerStopInteract(AInteractibleObjects* Interactible, AAPlayerCharacter* Player);

	// --- Nouveau : Système de relevage (hold) ---
	UFUNCTION(Server, Reliable)
	void ServerStartRevive(AAPlayerCharacter* Ally);

	UFUNCTION(Server, Reliable)
	void ServerCancelRevive();

	void CompleteRevive();

	TArray<AAPlayerCharacter*> GetNearbyPlayers(float Radius, bool bOnlyDead) const;


public:
	UPROPERTY()
	AInteractibleObjects* CurrentInteractible = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> InteractiblesAtRange;

	UPROPERTY(EditAnywhere)
	AAPlayerCharacter* PlayerCharacter;

	UPROPERTY(EditAnywhere)
	AAPlayerCharacter* AllyPlayer;

private:
	FTimerHandle ReviveTimer;

	UPROPERTY()
	AAPlayerCharacter* CurrentReviveTarget = nullptr;
};
