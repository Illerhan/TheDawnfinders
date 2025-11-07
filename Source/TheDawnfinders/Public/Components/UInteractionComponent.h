// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UInteractionComponent.generated.h"


UCLASS( ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent) )
class THEDAWNFINDERS_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInteractionComponent();


// Interactibles at range management
public :
	UFUNCTION()
	void AddInteractible(AActor* Interactible);

	UFUNCTION()
	void RemoveInteractible(AActor* Interactible);

	UFUNCTION(BlueprintCallable)
	AActor* GetNearestInteractible();


// Do & Stop Interaction
public :
	UFUNCTION(BlueprintCallable)
	void StartInteract();

	UFUNCTION(Server, Reliable)
	void ServerInteract(AInteractibleObjects* Interactible, AAPlayerCharacter* Player);

	UFUNCTION(BlueprintCallable)
	void StopInteract();

	UFUNCTION(Server, Reliable)
	void ServerStopInteract(AInteractibleObjects* Interactible, AAPlayerCharacter* Player);

	UFUNCTION()
	void TryInteract(AInteractibleObjects* InteractibleObject, AAPlayerCharacter* Player);


public :
	UPROPERTY()
	AInteractibleObjects* CurrentInteractible = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> InteractiblesAtRange;
};
