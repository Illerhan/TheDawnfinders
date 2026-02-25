// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "ANoise.generated.h"

UCLASS()
class THEDAWNFINDERS_API ANoise : public AActor
{
	GENERATED_BODY()

	ANoise();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public : 
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USphereComponent* NoiseZone;

	UPROPERTY(BlueprintReadWrite)
	AActor* NoiseOriginActor;

protected :
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bIsConstant;

	UPROPERTY()
	float Timer = 0.0f;
};
