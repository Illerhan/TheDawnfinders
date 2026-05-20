// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AkAudioEvent.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Activable.h"
#include "MovableObjects.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMovableMovementFinished);

UCLASS()
class THEDAWNFINDERS_API AMovableObjects : public AActor, public IActivable
{
	GENERATED_BODY()

public:
	AMovableObjects();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	


// === Interface ===
public :
	virtual void DoMainAction_Implementation() override;
	virtual void StopMainAction_Implementation() override;
	virtual int GetPlayerCount_Implementation() override;
	virtual void SetPlayerCount_Implementation(int Value) override;


// === Functions ===
public :
	UFUNCTION()
	void HandleProgress(float value);

	UFUNCTION()
	void OnTimeLineFinished();

	UFUNCTION()
	bool CanReverse() const;

	UFUNCTION()
	float GetTimelineProgress() const;

	UFUNCTION(Blueprintable,BlueprintCallable,BlueprintNativeEvent,Category="MovableObjects")
	void DoMovement();

	UFUNCTION()
	void DoReverseMovement();


// === Properties ===
public :
	UPROPERTY(EditAnywhere, BlueprintReadwrite)
	UStaticMesh* MovableMesh;

	UPROPERTY(EditAnywhere,Category = "Timeline")
	UCurveFloat* MoveCurve;
	
	FTimeline Timeline;

	FOnTimelineFloat TimelineProgress;
	
	FOnTimelineEvent TimelineFinished;
	
	UPROPERTY(BlueprintAssignable, Category = "MovableObjects|Events")
	FOnMovableMovementFinished OnMovementFinished;

	FVector StartPosition;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform",
		meta = (MakeEditWidget = true))
	FVector EndPosition;

	UPROPERTY(EditAnywhere, Category = "Timeline")
	mutable bool bCanMove = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline");
	float MovementDuration;

	UPROPERTY(BlueprintReadWrite)
	bool bIsMovingForward;

	UPROPERTY(BlueprintReadWrite,EditAnywhere)
	float CurrentTimelineProgress;
	float LastReverseTime;
	float ReverseCooldown;
	FVector FinalPosition;
	FVector OriginalStart;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int PlayerCount;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UAkAudioEvent* MovableSound;
	
	UPROPERTY()
	int32 MovableSoundID;
	
	UFUNCTION(NetMulticast, Reliable)
	void Multi_StoppingSound();
};
