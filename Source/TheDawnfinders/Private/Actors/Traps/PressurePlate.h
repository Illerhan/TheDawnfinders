// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "PressurePlate.generated.h"

UCLASS()
class THEDAWNFINDERS_API APressurePlate : public AActor
{
	GENERATED_BODY()

public:
	APressurePlate();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;


// === Functions ===
public :
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UPROPERTY(Blueprintable, EditAnywhere)
	UBoxComponent* BoxCollider;


// === Properties ====
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int NeededPlayerCount = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bActivatesWithPalanquin = false;

	UPROPERTY(Blueprintable ,EditAnywhere)
	TArray<AActor*> LinkedActors;

	UPROPERTY()
	int CurrentPlayerCount = 0;
};
