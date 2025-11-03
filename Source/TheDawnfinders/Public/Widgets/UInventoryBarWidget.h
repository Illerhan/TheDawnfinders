// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomStructs.h"
#include "UInventorySlotWidget.h"
#include "UInventoryBarWidget.generated.h"

class UInventoryComponent;

/**
 * 
 */
UCLASS()
class THEDAWNFINDERS_API UInventoryBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public :
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable, Category="Inventory")
	void ActualiseWidget(const TArray<FInventorySlot>& Slots, int32 CurrentIndex);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ShowWidget();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void HideWidget();

	
	
protected:
	virtual void NativeConstruct() override;
	
	void TryBindToInventory();

	virtual void NativeDestruct() override;

	UPROPERTY()
	UInventoryComponent* InventoryComponentRef;

	UPROPERTY(BlueprintReadWrite)
	TArray<UInventorySlotWidget*> InventorySlotsWidgets;

	FTimerHandle BindDelayTimerHandle;
		
};
