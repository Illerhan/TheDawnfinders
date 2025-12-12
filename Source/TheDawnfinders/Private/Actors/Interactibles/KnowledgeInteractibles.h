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
	AKnowledgeInteractibles();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	// === INTERFACE FUNCTIONS ===
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void StopInteract_Implementation(AActor* Interactor) override;
	
protected:
	UFUNCTION()
	virtual void BP_OnInteractionFinished_Implementation() override;

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float InteractionDuration;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int KnowledgeAmount;

};
