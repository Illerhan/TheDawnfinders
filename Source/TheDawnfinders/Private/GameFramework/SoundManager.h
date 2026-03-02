// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SoundManager.generated.h"

UCLASS()
class THEDAWNFINDERS_API ASoundManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASoundManager();

	
	UFUNCTION(Server, Reliable)
	void ServerPlaySound(USoundBase* Sound, FVector Location, float Loudness, float Range = 1000.f, bool bNeedNoise = false);

	UFUNCTION(NetMulticast, Reliable)
	void MultiPlaySound(USoundBase* Sound, FVector Location, float Loudness, float Range = 1000.f,bool bNeedNoise = false);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
