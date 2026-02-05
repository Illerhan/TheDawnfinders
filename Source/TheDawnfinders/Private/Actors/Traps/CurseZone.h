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
	ACurseZone();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION()
	void Initialise(float Radius, float CurseZoneDelay, float Duration);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Initialise(float Radius, float CurseZoneDelay);

	UFUNCTION()
	void InitialiseAfterDelay();

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void Multicast_SetEnabled(bool bEnable);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetEnabled(bool bEnable);

	UFUNCTION()
	void OnCurseOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnCurseOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	

protected :
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USphereComponent* CurseCollider;
	
	UPROPERTY(EditAnywhere)
	float CurseRadius;
	
	UPROPERTY(EditAnywhere)
	float CurseRate;

	UPROPERTY()
	float DestroyTimer;

	UPROPERTY()
	bool DestroyAfterTimer;

	FTimerHandle StartCurseDelayHandle;
};
