// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Traps/ATrapBase.h"
#include "ASpikeTrap.generated.h"

UCLASS()
class THEDAWNFINDERS_API AASpikeTrap : public ATrapBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AASpikeTrap();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void DoTrapAction() override;
};
