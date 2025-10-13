// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomStructs.h"
#include "UInventorySlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class THEDAWNFINDERS_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public :
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ActualiseVisuals(FInventorySlot Data, bool IsSelected);
};
