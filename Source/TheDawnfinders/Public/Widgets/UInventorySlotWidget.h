// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomStructs.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "UInventorySlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class THEDAWNFINDERS_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public :

	UPROPERTY(BlueprintReadWrite, meta =(BindWidget))
	UImage* IconImage;

	UPROPERTY(BlueprintReadWrite, meta =(BindWidget))
	UCanvasPanel* CanvasPanel;

	//UPROPERTY(BlueprintReadWrite, meta =(BindWidget))
	//UWidgetAnimation* Select;

	UPROPERTY(BlueprintReadWrite, Category="InventorySlot")
	bool bISlotSelected = false;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ActualiseVisuals(FInventorySlot Data, bool IsSelected);
	virtual void ActualiseVisuals_Implementation(FInventorySlot Data, bool IsSelected);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void UpdateVisuals(FInventorySlot Data, bool IsSelected);
};
