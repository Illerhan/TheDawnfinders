// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactibles/Carriable.h"
#include "Components/ActorComponent.h"
#include "UInteractionComponent.generated.h"

class UAkAudioEvent;
class AAPlayerCharacter;
class UQTEWidget;


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

	UFUNCTION(BlueprintCallable)
	void TryInteract(AActor* Interactible, AAPlayerCharacter* Player);

	UFUNCTION(Server, Reliable)
	void ServerInteract(AActor* Interactible, AAPlayerCharacter* Player);

	UFUNCTION(BlueprintCallable)
	void StopInteract();

	UFUNCTION(BlueprintCallable)
	void CancelInteraction();

	UFUNCTION(Server, Reliable)
	void ServerStopInteract(AActor* Interactible, AAPlayerCharacter* Player);

	UFUNCTION(Client, Reliable)
	void ClientStopInteract(AActor* Interactible, AAPlayerCharacter* Player);

	void StartExternalQTE(AActor* QTEActor);
	
	UFUNCTION(Client,Unreliable)
	void PlayInteractSound();


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


// === QTE ===
public :
	UFUNCTION(BlueprintCallable)
	void StartRotativeQTE(AInteractibleObjects* Interactible);
	
	UFUNCTION(BlueprintCallable)
	void StartMashButtonQTE(AInteractibleObjects* Interactible);


public : 
	UFUNCTION(BlueprintCallable)
	void DoInteractAnimation(AActor* Target);

	UFUNCTION(BlueprintCallable)
	void EndInteractAnimatiopn();


// === GETTERS ===
public : 
	UFUNCTION(BlueprintCallable)
	AActor* GetCurrentInteractible() 
	{
		if (!bIsInInteraction) return nullptr;

		if(CurrentInteractible)
			return CurrentInteractible;

		if (InteractingQTEActor)
			return InteractingQTEActor;

		return (AActor*)CurrentHelpedTarget;
	}

	UFUNCTION(BlueprintCallable)
	AActor* GetCurrentAnimInteractible()
	{
		if (!bIsInInteraction) return nullptr;
		return CurrentAnimInteractible;
	}


// === PUBLIC PROPERTIES ===
public:
	UPROPERTY(Replicated)
	AActor* CurrentInteractible = nullptr;

	UPROPERTY(Replicated)
	AActor* CurrentAnimInteractible = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> InteractiblesAtRange;

	UPROPERTY(BlueprintReadOnly)
	TArray<AAPlayerCharacter*> PlayersAtRange;

	UPROPERTY(EditAnywhere)
	AAPlayerCharacter* PlayerCharacter;

	UPROPERTY(EditAnywhere)
	AAPlayerCharacter* AllyPlayer;


// === PRIVATE PROPERTIES ===
public:
	UPROPERTY(Replicated, EditAnywhere,BlueprintReadWrite,Blueprintable)
	ACarriable* CarriedItem;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool bIsInInteraction;
	
private:
	UPROPERTY()
	FTimerHandle HelpTimer;

	UPROPERTY()
	AActor* InteractingQTEActor;

	UPROPERTY()
	UQTEWidget* CurrentQTEWidget;
	
	UPROPERTY()
	bool bIsDoingQTE;

	UPROPERTY(Replicated)
	AActor* NearestInteractible;

	UPROPERTY(Replicated)
	AAPlayerCharacter* CurrentHelpedTarget = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_HelpState)
	bool bIsHelping = false;

	UPROPERTY(ReplicatedUsing = OnRep_HelpState)
	bool bWasCrouched = false;

	UPROPERTY()
	float HelpDuration = 4.f;

	UPROPERTY(ReplicatedUsing = OnRep_HelpState)
	float HelpTimeRemaining = 0.f;

public:
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UAkAudioEvent* InteractSound;
	
	UPROPERTY()
	int32 InteractSoundID;
	
};	