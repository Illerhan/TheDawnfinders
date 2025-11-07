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
	// Sets default values for this actor's properties
	ALever();
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Interactibles")
	TArray<AMovableObjects*> LinkedObjects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lever")
	bool bRequiresHold = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void Interaction(AAPlayerCharacter* Player) override;
	void StartHoldInteraction(AAPlayerCharacter* Player);
	virtual void StopInteraction(AAPlayerCharacter* Player) override;
	void StopHoldInteraction(AAPlayerCharacter* Player);


private:
	bool bIsBeingHeld;
};
