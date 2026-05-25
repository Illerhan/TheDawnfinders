// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AkAudioEvent.h"
#include "MovableObjects.h"
#include "Doors.generated.h"

UCLASS()
class THEDAWNFINDERS_API ADoors : public AMovableObjects
{
	GENERATED_BODY()

public:
	ADoors();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;


// === Interface ===
public:
	virtual void DoMainAction_Implementation() override;
	virtual void StopMainAction_Implementation() override;


// === Functions ===
public :
	virtual void OnTimelineFinished();
	UFUNCTION(BlueprintCallable, Category = "Doors")
	void CloseDoor();

	UFUNCTION(BlueprintCallable, Category = "Doors")
	virtual void StartOpening();

	UFUNCTION(BlueprintCallable, Category = "Doors")
	void AddOpeningPlayer();

	UFUNCTION(BlueprintCallable, Category = "Doors")
	void RemoveOpeningPlayer();

	UFUNCTION(BlueprintCallable, Category = "Doors")
	void PauseOpening();

	UFUNCTION(BlueprintCallable, Category = "Doors")
	virtual void StopOpening();
	
	UFUNCTION(BlueprintCallable, Category = "Door")
	void OpenPermanently();


// === Properties ===
public :
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Doors")
	bool bIsClosed = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Doors")
	bool bIsFullyOpen;

	UPROPERTY(EditAnywhere, Blueprintable, Category = "Doors")
	bool bNeedHold;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Settings")
	bool bIsPermanentlyOpen = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Settings")
	bool bIsExtractionDoor = false;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Settings")
	bool bAutoCloseWhenFullyOpen = true;
	
	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category="Door")
	float AutoCloseDelay = 2.0f;

	FTimerHandle AutoCloseTimer;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Settings")
	bool bStartsOpen = true;


protected : 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Doors")
	int NeededTriggerCount = 1;

	UPROPERTY(BlueprintReadOnly)
	int CurrentTriggerCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Doors")
	float SpeedAddedPerAdditionalPlayer = 1;

	UPROPERTY(BlueprintReadOnly)
	float CurrentAddedSpeed = 1;
	
	UFUNCTION(NetMulticast,Unreliable, Category = "Doors")
	virtual void Multi_OpeningSound();

	
};
