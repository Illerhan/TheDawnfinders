// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactibles/Interactible.h"
#include "KnowledgeInteractibles.generated.h"

UCLASS()
class THEDAWNFINDERS_API AKnowledgeInteractibles : public AInteractibleObjects
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AKnowledgeInteractibles();
	
	// === INTERFACE FUNCTIONS ===
	
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void StopInteract_Implementation(AActor* Interactor) override;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void BP_OnInteractionFinished_Implementation() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float InteractionDuration;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int KnowledgeAmount;

};
