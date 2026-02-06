// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Traps/ATrapBase.h"
#include "GameFramework/SoundManager.h"
#include "NoiseTrap.generated.h"

UCLASS()
class THEDAWNFINDERS_API ANoiseTrap : public ATrapBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANoiseTrap();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void DoTrapAction(AActor* OtherActor = nullptr) override;
	UPROPERTY()
	ASoundManager* SoundManagerInstance;
};
