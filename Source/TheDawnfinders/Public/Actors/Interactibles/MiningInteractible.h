// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactibles/Interactible.h"
#include "MiningInteractible.generated.h"

/**
 * 
 */
UCLASS()
class THEDAWNFINDERS_API AMiningInteractible : public AInteractibleObjects
{
	GENERATED_BODY()

public :
	virtual void Interact_Implementation(AActor* Interactor) override;

protected :
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MinItemToSpawnCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxItemToSpawnCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ItemToSpawn;
};
