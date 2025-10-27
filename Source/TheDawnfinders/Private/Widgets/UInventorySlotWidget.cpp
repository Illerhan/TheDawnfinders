// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/UInventorySlotWidget.h"

void UInventorySlotWidget::ActualiseVisuals_Implementation(FInventorySlot Data, bool IsSelected)
{
	bISlotSelected = IsSelected;
	UE_LOG(LogTemp, Log, TEXT("ActualiseVisuals C++ - IsSelected: %s, bISlotSelected: %s, HasItemData: %s"), 
		   IsSelected ? TEXT("true") : TEXT("false"),
		   bISlotSelected ? TEXT("true") : TEXT("false"),
		   Data.ItemData ? TEXT("true") : TEXT("false"));
	UpdateVisuals(Data, IsSelected);
}

void UInventorySlotWidget::UpdateVisuals_Implementation(FInventorySlot Data, bool IsSelected)
{
}

