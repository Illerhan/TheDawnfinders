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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bIsLoud;

	UPROPERTY()
	float Radius = 0.0f;

protected :
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bIsConstant;

public:
	void SetIsConstant(const bool bConstant)
	{
		this->bIsConstant = bConstant;
	}

protected:
	UPROPERTY()
	float Timer = 0.0f;
};
