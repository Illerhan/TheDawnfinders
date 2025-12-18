// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactible.h"
#include "Lock.generated.h"

class UItemData;
class APlayerCharacter;
class AMovableObjects;


UCLASS()
class THEDAWNFINDERS_API ALock : public AInteractibleObjects
{
	GENERATED_BODY()

public :
	ALock();
	virtual void Tick(float DeltaTime) override;

	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void StopInteract_Implementation(AActor* Interactor) override;

	UFUNCTION()
	virtual void BP_OnInteractionFinished_Implementation() override;



// === PUBLIC PROPERTIES ===
public :
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UItemData* NeededKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AMovableObjects*> LinkedObjects;


// === PRIVATE PROPERTIES ===

};
