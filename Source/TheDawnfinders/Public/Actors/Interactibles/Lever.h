// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactible.h"
#include "Actors/MovableObjects/MovableObjects.h"
#include "Actors/MovableObjects/Doors.h"
#include "Lever.generated.h"

UCLASS()
class THEDAWNFINDERS_API ALever : public AInteractibleObjects
{
	GENERATED_BODY()

public:
	ALever();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;


public :
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Interactibles")
	TArray<AMovableObjects*> LinkedObjects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lever")
	bool bRequiresHold = false;

	virtual void Interact_Implementation(AActor* Interactor) override;
	void StartHoldInteraction(AActor* Player);
	virtual void StopInteract_Implementation(AActor* Interactor) override;
	void StopHoldInteraction(AActor* Player);


private:
	bool bIsBeingHeld;
};
