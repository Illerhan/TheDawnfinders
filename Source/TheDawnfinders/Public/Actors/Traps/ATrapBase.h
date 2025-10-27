// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"

#include "ATrapBase.generated.h"

UCLASS()
class THEDAWNFINDERS_API ATrapBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATrapBase();
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Trap")
	int Damages;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Trap")
	float Cooldown;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Trap")
	bool bEnable;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Collision")
	UBoxComponent* TrapCollider;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Collision")
	USoundBase* Sound;

	UFUNCTION(BlueprintCallable)
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
						bool bFromSweep, const FHitResult& SweepResult);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable,Category="Trap")
	void DisableTrap() {bEnable = false;};
	
	UFUNCTION(BlueprintCallable,Category="Trap")
	virtual void DoTrapAction() {return;};

	
};
