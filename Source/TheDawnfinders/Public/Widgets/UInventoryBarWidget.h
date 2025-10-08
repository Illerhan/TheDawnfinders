// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomStructs.h"
#include "UInventoryBarWidget.generated.h"

class UInventorySlotWidget;

/**
 * 
 */
UCLASS()
class THEDAWNFINDERS_API UInventoryBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public :
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ActualiseWidget(const TArray<FInventorySlot>& Slots, int CurrentIndex);

protected :
	UPROPERTY(BlueprintReadWrite)
	TArray<UInventorySlotWidget*> InventorySlotsWidgets;
};
