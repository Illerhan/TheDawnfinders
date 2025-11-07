// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerLightComponent.h"
#include "Actors/Interactibles/Interactible.h"
#include "BurnableObjects.generated.h"

UCLASS()
class THEDAWNFINDERS_API ABurnableObjects : public AInteractibleObjects
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABurnableObjects();

	UPROPERTY(BlueprintReadWrite,EditAnywhere,Category="BurnableObjects")
	UPlayerLightComponent* LightComponent;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
