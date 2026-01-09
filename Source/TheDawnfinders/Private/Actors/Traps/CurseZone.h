// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CurseZone.generated.h"

class USphereComponent;

UCLASS()
class THEDAWNFINDERS_API ACurseZone : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACurseZone();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION()
	void OnCurseOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnCurseOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UPROPERTY(EditAnywhere)
	USphereComponent* CurseCollider;
	
	UPROPERTY(EditAnywhere)
	float CurseRadius;
	
	UPROPERTY(EditAnywhere)
	float CurseRate;
};
