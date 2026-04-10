// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DangerManager.generated.h"

UCLASS()
class THEDAWNFINDERS_API ADangerManager : public AActor
{
	GENERATED_BODY()

public:
	ADangerManager();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void IncreaseDangerLevel(float DangerCost);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void MuleCalled();
	
};
