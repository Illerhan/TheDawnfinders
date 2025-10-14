// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Actor.h"
#include "MovableObjects.generated.h"

UCLASS()
class THEDAWNFINDERS_API AMovableObjects : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMovableObjects();
	UPROPERTY(EditAnywhere,BlueprintReadwrite)
	UStaticMesh* MovableMesh;

	UPROPERTY(EditAnywhere, Category="Timeline")
	UCurveFloat* MoveCurve;

	FTimeline Timeline;
	FOnTimelineFloat TimelineProgress;
	FOnTimelineEvent TimelineFinished;

	FVector StartPosition;
	UPROPERTY(Blueprintable,BlueprintReadWrite,EditAnywhere,Category="Timeline")
	FVector EndPosition;

	UPROPERTY(EditAnywhere, Category="Timeline")
	bool bCanMove = true;

	UFUNCTION()
	void HandleProgress(float value);

	UFUNCTION()
	void OnTimeLineFinished();

	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(Blueprintable)
	virtual void DoMovement();
};
