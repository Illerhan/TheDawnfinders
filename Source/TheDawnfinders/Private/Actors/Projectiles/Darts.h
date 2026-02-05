// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "Darts.generated.h"

UCLASS()
class THEDAWNFINDERS_API ADarts : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADarts();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, Blueprintable)
	USphereComponent* SphereCollider; 
	
	UPROPERTY(Blueprintable, EditAnywhere)
	class UProjectileMovementComponent* MoveComp;
	
	bool bCooldown;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UPROPERTY(Replicated, EditAnywhere,Blueprintable)
	float Cooldown;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
