// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactible.h"
#include "ZiplineInteractible.generated.h"

UCLASS()
class THEDAWNFINDERS_API AZiplineInteractible : public AInteractibleObjects
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AZiplineInteractible();

	UPROPERTY(Blueprintable)
	bool bLinked = false;

	UPROPERTY(Blueprintable)
	AZiplineInteractible* LinkedZipline = nullptr;

	UPROPERTY(Blueprintable)
	float ZiplineRange;

	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void CheckZiplineInteractibleInRange();

	bool IsInLigneOfSight(AZiplineInteractible* otherZipline);

};
