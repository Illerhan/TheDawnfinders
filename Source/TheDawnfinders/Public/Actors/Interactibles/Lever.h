// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactible.h"
#include "Actors/MovableObjects/MovableObjects.h"
#include "Actors/MovableObjects/Doors.h"
#include "Interfaces/Toggleable.h"
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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lever|Toggleables")
	TArray<AActor*> LinkedToggleables;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lever")
	bool bRequiresHold = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lever", meta = (min = 1, max = 4))
	int HoldPlayerCountNeeded = 1;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DoPlayerAutoMove(AAPlayerCharacter* Player);

	virtual bool GetCanBeUsed_Implementation(AActor* Interactor) override;

	virtual void Interact_Implementation(AActor* Interactor) override;
	void StartHoldInteraction(AActor* Player);
	virtual void StopInteract_Implementation(AActor* Interactor) override;
	void StopHoldInteraction(AActor* Player);


protected:
	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bIsOn;

	UPROPERTY()
	bool bIsBeingHeld;

	UPROPERTY()
	int HoldPlayerCount;
};
