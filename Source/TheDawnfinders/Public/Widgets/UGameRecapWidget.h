// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UGameRecapWidget.generated.h"

/**
 * 
 */
UCLASS()
class THEDAWNFINDERS_API UGameRecapWidget : public UUserWidget
{
	GENERATED_BODY()
	
public :
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void DisplaySuccessRecap();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void DisplayFailRecap();

	UPROPERTY(BlueprintReadOnly)
	bool bIsDisplayed;
};
