// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactible.h"
#include "Actors/MovableObjects/MovableObjects.h"
#include "Lever.generated.h"

UCLASS()
class THEDAWNFINDERS_API ALever : public AInteractibleObjects
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALever();

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Interactibles")
	AMovableObjects* LinkedObject;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void Interaction_Implementation() override;
};
