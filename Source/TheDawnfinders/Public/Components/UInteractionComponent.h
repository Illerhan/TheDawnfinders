// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UInteractionComponent.generated.h"

class AAPlayerCharacter;


UCLASS(ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent))
class THEDAWNFINDERS_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	// === INTERCACTIBLES AT RANGE MANAGEMENT ===
public:
	UFUNCTION()
	void AddInteractible(AActor* Interactible);

	UFUNCTION()
	void RemoveInteractible(AActor* Interactible);

	UFUNCTION(BlueprintCallable)
	AActor* GetNearestInteractible();


// === DO & STOP INTERACTION ===
public:
	UFUNCTION()
	void TryInteractAlly(AAPlayerCharacter* Ally, AAPlayerCharacter* Player);

	UFUNCTION(BlueprintCallable)
	void StartInteract();

	UFUNCTION()
	void TryInteract(AActor* Interactible, AAPlayerCharacter* Player);

	UFUNCTION(Server, Reliable)
	void ServerInteract(AActor* Interactible, AAPlayerCharacter* Player);

	UFUNCTION(BlueprintCallable)
	void StopInteract();

	UFUNCTION(Server, Reliable)
	void ServerStopInteract(AActor* Interactible, AAPlayerCharacter* Player);


// === REVIVE ===
public:
	UFUNCTION(Server, Reliable)
	void ServerStartRevive(AAPlayerCharacter* Ally);

	UFUNCTION(Server, Reliable)
	void ServerCancelRevive();

	UFUNCTION()
	void CompleteRevive();

	UFUNCTION(BlueprintCallable)
	TArray<AAPlayerCharacter*> GetNearbyPlayers(float Radius, bool bOnlyDead) const;

	UPROPERTY(ReplicatedUsing=OnRep_ReviveState)
	bool bIsReviving = false;
	float ReviveDuration = 2.f;
	UPROPERTY(ReplicatedUsing=OnRep_ReviveState)
	float ReviveTimeRemaining = 0.f;
	
	UFUNCTION(Client, Reliable)
	void Client_ShowReviveProgress(float Duration);

	UFUNCTION(Client, Reliable)
	void Client_HideReviveProgress();
	
	UFUNCTION()
	void OnRep_ReviveState();
	
	


// === PUBLIC PROPERTIES ===
public:
	UPROPERTY()
	AActor* CurrentInteractible = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> InteractiblesAtRange;

	UPROPERTY(EditAnywhere)
	AAPlayerCharacter* PlayerCharacter;

	UPROPERTY(EditAnywhere)
	AAPlayerCharacter* AllyPlayer;


// === PRIVATE PROPERTIES ===
private:
	UPROPERTY()
	FTimerHandle ReviveTimer;

	UPROPERTY()
	AActor* InteractingQTEActor;

	UPROPERTY()
	AAPlayerCharacter* CurrentReviveTarget = nullptr;
};
