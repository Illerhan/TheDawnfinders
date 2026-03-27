// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Activable.generated.h"

UINTERFACE()
class UActivable : public UInterface
{
	GENERATED_BODY()
};


class THEDAWNFINDERS_API IActivable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void DoMainAction();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void StopMainAction();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int GetPlayerCount();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetPlayerCount(int Value);
};
