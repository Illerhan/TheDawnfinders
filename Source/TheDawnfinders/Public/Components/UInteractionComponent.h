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


// === INTERACTIBLES AT RANGE MANAGEMENT ===
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

	UFUNCTION(BlueprintCallable)
	void CancelInteraction();

	UFUNCTION(Server, Reliable)
	void ServerStopInteract(AActor* Interactible, AAPlayerCharacter* Player);

	void StartExternalQTE(AActor* QTEActor);


// === CARRY ===
public : 
	UFUNCTION(BlueprintCallable)
	void StartCarryHeavyItem(ACarriable* Item);

	UFUNCTION(BlueprintCallable)
	void PutInHeavyItem(AActor* Target);
	
	UFUNCTION(Server, Reliable)
	void Server_PutInHeavyItem(AActor* Target);
	

	UFUNCTION(BlueprintCallable)
	void EndCarryHeavyItem();


// === REVIVE ===
public:
	UFUNCTION(Server, Reliable)
	void ServerStartHelp(AAPlayerCharacter* Ally);

	UFUNCTION(Server, Reliable)
	void ServerCancelHelp();

	UFUNCTION()
	void CompleteHelp();

	UFUNCTION(BlueprintCallable)
	TArray<AAPlayerCharacter*> GetNearbyPlayers(float Radius, bool bOnlyDead) const;
	
	UFUNCTION(Client, Reliable)
	void Client_ShowHelpProgress(float Duration);

	UFUNCTION(Client, Reliable)
	void Client_HideHelpProgress();
	
	UFUNCTION()
	void OnRep_HelpState();
	
	


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
public:
	UPROPERTY(Replicated, EditAnywhere, Blueprintable)
	ACarriable* CarriedItem;
	
private:
	UPROPERTY()
	FTimerHandle HelpTimer;

	UPROPERTY()
	AActor* InteractingQTEActor;
	
	UPROPERTY()
	bool bIsDoingQTE;

	UPROPERTY()
	AAPlayerCharacter* CurrentHelpedTarget = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_HelpState)
	bool bIsHelping = false;

	UPROPERTY()
	float HelpDuration = 4.f;

	UPROPERTY(ReplicatedUsing = OnRep_HelpState)
	float HelpTimeRemaining = 0.f;
};
