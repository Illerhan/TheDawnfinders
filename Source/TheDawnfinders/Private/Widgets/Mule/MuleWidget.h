// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MuleWidget.generated.h"

/**
 * 
 */
UCLASS()
class THEDAWNFINDERS_API UMuleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Mule)
	void UpdateMuleWidget(int Charges, float ChargesCooldown, float CallCooldown, int GoldAmount);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Mule)
	void DisplayWidget();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Mule)
	void HideWidget();
};
