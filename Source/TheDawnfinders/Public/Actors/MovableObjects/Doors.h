// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovableObjects.h"
#include "Doors.generated.h"

UCLASS()
class THEDAWNFINDERS_API ADoors : public AMovableObjects
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADoors();

	UPROPERTY(EditAnywhere, Blueprintable, Category = "Doors")
	bool bNeedHold;

	UFUNCTION(BlueprintCallable, Category = "Doors")
	void StartOpening();

	UFUNCTION(BlueprintCallable, Category = "Doors")
	void StopOpening();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category ="Doors")
	bool bIsFullyOpen;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category ="Doors")
	bool bIsClosed = true;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;
	virtual void OnTimelineFinished();

public:
	// Called every frame

};
