// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UWorldProgressBar.generated.h"


UCLASS()
class THEDAWNFINDERS_API UWorldProgressBar : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Show(float CurrentValue);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ActualiseProgress(float CurrentValue);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Hide();

private :
	UPROPERTY()
	float StartValue;
};
